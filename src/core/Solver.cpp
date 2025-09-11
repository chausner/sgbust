#include "core/Solver.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <execution>
#include <format>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <mutex>
#include <numeric>
#include <ranges>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

#include "core/CompactGrid.h"
#include "core/MemoryUsage.h"

namespace sgbust
{
    std::optional<SolverResult> Solver::Solve(const Grid& grid, unsigned int minGroupSize, const Scoring& scoring, const Solution& solutionPrefix)
    {
        this->minGroupSize = minGroupSize;
        this->scoring = &scoring;
        this->solutionPrefix = solutionPrefix;

        Grid gridWithPrefix = grid;

        if (!solutionPrefix.IsEmpty())
            gridWithPrefix.ApplySolution(solutionPrefix, minGroupSize);

        Score initialScore = scoring.CreateScore(gridWithPrefix, minGroupSize);

        grids.clear();
        grids[initialScore].insert(CompactGrid(gridWithPrefix));

        solution = Solution();
        bestScore = std::nullopt;
        solutionGrid = std::nullopt;
        beamSize = 1;
        multiplier = 0;

        bool stop = false;

        if (!gridWithPrefix.HasGroups(minGroupSize))
            CheckSolution(gridWithPrefix, initialScore, stop);

        for (depth = 0; depth < MaxDepth || !MaxDepth; depth++)
        {
            if (!Quiet)
                PrintStats();

            if (TrimmingEnabled)
                TrimBeam();

            SolveDepth(MaxDepth && depth == *MaxDepth - 1, stop);

            if (stop)
                break;
        }

        if (bestScore.has_value())
            return SolverResult{ *bestScore, std::move(solution), std::move(*solutionGrid) };
        else
            return std::nullopt;
    }

    void Solver::PrintStats() const
    {
        auto [minScore, maxScore] = std::ranges::minmax(grids | std::views::transform([](const auto& b) { return b.first.Value; }));
        long long scoreSum = std::transform_reduce(grids.begin(), grids.end(), 0LL, std::plus<>(), [](auto& b) { return b.first.Value * b.second.size(); });
        double avgScore = static_cast<double>(scoreSum) / beamSize;

        std::string output = std::format(
            "Depth: {:3}, grids: {:9}, hash sets: {:4}, scores (min/avg/max): {}/{:.1f}/{}",
            depth,
            beamSize,
            grids.size(),
            minScore,
            avgScore,
            maxScore
        );

        std::optional<std::size_t> memoryUsage = GetCurrentMemoryUsage();
        if (memoryUsage.has_value())
            output += std::format(", memory: {}MB", (*memoryUsage / 1024 / 1024));

        std::cout << output << std::endl;
    }

    void Solver::PrintProgress(const std::map<Score, GridHashSet>& newGrids, unsigned int gridsSolved, unsigned int newBeamSize) const
    {
        int curMinScore = 0;
        int curMaxScore = 0;
        double curAvgScore = 0.0;

        std::shared_lock lock(mutex);
        std::size_t numHashSets = newGrids.size();

        if (!newGrids.empty())
        {
            auto [minScore, maxScore] = std::ranges::minmax(newGrids | std::views::transform([](const auto& b) { return b.first.Value; }));
            curMinScore = minScore;
            curMaxScore = maxScore;
            long long scoreSum = std::transform_reduce(newGrids.begin(), newGrids.end(), 0LL, std::plus<>(), [](auto& b) { return b.first.Value * b.second.size(); });
            curAvgScore = static_cast<double>(scoreSum) / newBeamSize;
        }

        lock.unlock();

        double percentProcessed = gridsSolved * 100.0 / beamSize;
        double percentBeamSizeLimit = 0.0;
        if (MaxBeamSize.has_value())
            percentBeamSizeLimit = newBeamSize * 100.0 / *MaxBeamSize;
        double progress = std::max(percentProcessed, percentBeamSizeLimit);

        std::string output = std::format(
            "Depth: {:3}, grids: {:9}, hash sets: {:4}, scores (min/avg/max): {}/{:.1f}/{}",
            depth + 1,
            newBeamSize,
            numHashSets,
            curMinScore,
            curAvgScore,
            curMaxScore
        );

        std::optional<std::size_t> memoryUsage = GetCurrentMemoryUsage();
        if (memoryUsage.has_value())
            output += std::format(", memory: {}MB", (*memoryUsage / 1024 / 1024));

		output += std::format(" ({:.1f}%)", progress);

        std::cout << "\x1b[2K\r" + output << std::flush;
    }

    void Solver::ClearProgress() const
    {
        std::cout << "\x1b[2K\r" << std::flush;
    }

    void Solver::SolveDepth(bool maxDepthReached, bool& stop)
    {
        std::map<Score, GridHashSet> newGrids;

        std::atomic_uint gridsSolved = 0;
        std::atomic_uint newBeamSize = 0;

        std::optional<std::jthread> reporter;

        if (!Quiet)
			reporter.emplace([&](std::stop_token stopToken) {
                std::mutex reporterMutex;
                std::condition_variable reporterCv;
                std::stop_callback callback(stopToken, [&]() { reporterCv.notify_all(); });
            
                while (true)
                {
                    {
                        std::unique_lock lock(reporterMutex);
                        if (reporterCv.wait_for(lock, std::chrono::seconds(1), [&]() { return stopToken.stop_requested(); }))
                            break;
                    }

					PrintProgress(newGrids, gridsSolved, newBeamSize);
                }

                ClearProgress();
            });

        for (auto it = grids.begin(); it != grids.end(); it = grids.erase(it))
        {
            auto& [score, hashSet] = *it;

            // std::for_each is a little bit faster here than hashSet.for_each but only MSVC supports parallel execution for it,
            // therefore we fall back to hashSet.for_each for other compilers
#ifdef _MSC_VER
            std::for_each(std::execution::par, hashSet.begin(), hashSet.end(), [&](const CompactGrid& grid) {
#else
            hashSet.for_each(std::execution::par, [&](const CompactGrid& grid) {
#endif
                if (stop || (MaxBeamSize && newBeamSize >= MaxBeamSize))
                    return;

                newBeamSize += SolveGrid(grid.Expand(), score, newGrids, maxDepthReached, stop);

                gridsSolved++;

                // overall, deallocation is faster if we deallocate the data inside CompactGrids here already
                const_cast<CompactGrid&>(grid) = CompactGrid();
            });

            if (stop || (MaxBeamSize && newBeamSize >= MaxBeamSize))
                break;
        }

        if (reporter.has_value())
        {
            reporter->request_stop();
            reporter->join();
        }

        multiplier = static_cast<double>(newBeamSize) / gridsSolved;

        if (newBeamSize == 0)
            stop = true;

        grids = std::move(newGrids);
        beamSize = newBeamSize;
    }

    unsigned int Solver::SolveGrid(const Grid& grid, Score score, std::map<Score, GridHashSet>& newGrids, bool maxDepthReached, bool& stop)
    {
        static thread_local std::vector<Group> groups;
        grid.GetGroups(groups, minGroupSize);

        unsigned int numNewGridsInserted = 0;

        auto getOrCreateHashSet = [&](const Score& score) -> GridHashSet& {
            {
                std::shared_lock lock(mutex);
                auto it = newGrids.find(score);                
                if (it != newGrids.end())
                    return it->second;
            }
            {
                std::unique_lock lock(mutex);
                return newGrids[score];
            }
        };

        for (int i = 0; i < groups.size(); i++)
        {
            Grid newGrid(grid.Width, grid.Height, grid.Blocks.get(), grid.Solution.Append(i));
            newGrid.RemoveGroup(groups[i]);

            Score newScore = scoring->RemoveGroup(score, grid, groups[i], newGrid, minGroupSize);

            if (!newGrid.HasGroups(minGroupSize))
                CheckSolution(newGrid, newScore, stop);
            else
                if (!maxDepthReached)
                {
                    auto [it, inserted] = getOrCreateHashSet(newScore).insert(CompactGrid(std::move(newGrid)));
                    if (inserted)
                        numNewGridsInserted++;
                }
        }

        return numNewGridsInserted;
    }

    void Solver::CheckSolution(const Grid& grid, Score score, bool& stop)
    {
        if (!stop && (!bestScore.has_value() || score.Value < *bestScore))
        {
            std::scoped_lock lock(mutex);

            if (!stop && (!bestScore.has_value() || score.Value < *bestScore))
            {
                bestScore = score.Value;
                solution = solutionPrefix.Append(grid.Solution);
                solutionGrid = grid;
                solutionGrid->Solution = solution;

                if (scoring->IsPerfectScore(score))
                    stop = true;
            }
        }
    }

    void Solver::TrimBeam()
    {
        if (MaxBeamSize && multiplier > 1)
        {
            unsigned int reducedBeamSize = std::ceil(*MaxBeamSize / multiplier * TrimmingSafetyFactor);

            if (beamSize > reducedBeamSize)
            {
                unsigned int accumulatedSize = 0;

                auto it = grids.begin();
                for (; ; it++)
                {
                    accumulatedSize += it->second.size();
                    if (accumulatedSize >= reducedBeamSize)
                        break;
                }

                GridHashSet& hashSet = it->second;
                auto it2 = hashSet.begin();
                std::advance(it2, accumulatedSize - reducedBeamSize);
                hashSet.erase(hashSet.begin(), it2);

                it++;
                grids.erase(it, grids.end());

                beamSize = reducedBeamSize;
            }
        }
    }
}