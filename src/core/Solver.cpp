#include "core/Solver.h"

#include <atomic>
#include <cmath>
#include <execution>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <ranges>
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

        Grid bg = grid;

        if (!solutionPrefix.IsEmpty())
            bg.ApplySolution(solutionPrefix, minGroupSize);

        grids.clear();
        grids[scoring.CreateScore(bg, minGroupSize)].insert(CompactGrid(bg));

        solution = Solution();
        bestScore = std::numeric_limits<int>::max();
        solutionGrid = std::nullopt;
        dbSize = 1;
        multiplier = 0;

        bool stop = false;

        for (depth = 0; depth < MaxDepth || !MaxDepth; depth++)
        {
            if (!Quiet)
                PrintStats();

            if (TrimDB)
                TrimDatabase();

            SolveDepth(stop, DontAddToDBLastDepth && MaxDepth && depth == *MaxDepth - 1);

            if (stop)
                break;
        }

        if (bestScore != std::numeric_limits<int>::max())
            return SolverResult{ bestScore, std::move(solution), std::move(*solutionGrid) };
        else
            return std::nullopt;
    }

    void Solver::PrintStats() const
    {
        auto [minScore, maxScore] = std::ranges::minmax(grids | std::views::transform([](const auto& b) { return b.first.Value; }));
        long long scoreSum = std::transform_reduce(grids.begin(), grids.end(), 0LL, std::plus<>(), [](auto& b) { return b.first.Value * b.second.size(); });
        double avgScore = static_cast<double>(scoreSum) / dbSize;
        std::cout <<
            "Depth: " << std::setw(3) << depth <<
            ", grids: " << std::setw(9) << dbSize <<
            ", hash sets: " << std::setw(4) << grids.size() <<
            ", scores (min/avg/max): " << minScore << "/" << std::fixed << std::setprecision(1) << avgScore << "/" << maxScore;

        std::optional<std::size_t> memoryUsage = GetCurrentMemoryUsage();
        if (memoryUsage.has_value())
            std::cout << ", memory: " << (*memoryUsage / 1024 / 1024) << "MB";

        std::cout << std::endl;
    }

    void Solver::SolveDepth(bool& stop, bool dontAddToDB)
    {
        std::map<Score, GridHashSet> newGrids;

        std::atomic_uint gridsSolved = 0;
        std::atomic_uint newDBSize = 0;

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
                if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
                    return;

                newDBSize += SolveGrid(grid.Expand(), score, newGrids, stop, dontAddToDB);

                gridsSolved++;

                // overall, deallocation is faster if we deallocate the data inside CompactGrids here already
                const_cast<CompactGrid&>(grid) = CompactGrid();
                });

            if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
                break;
        }

        multiplier = static_cast<double>(newDBSize) / gridsSolved;

        if (newDBSize == 0)
            stop = true;

        grids = std::move(newGrids);
        dbSize = newDBSize;
    }

    unsigned int Solver::SolveGrid(const Grid & grid, Score score, std::map<Score, GridHashSet>&newGrids, bool& stop, bool dontAddToDB)
    {
        static thread_local std::vector<Group> groups;
        grid.GetGroups(groups, minGroupSize);

        if (groups.empty())
            CheckSolution(score, grid, stop);

        int c = 0;

        for (int i = 0; i < groups.size(); i++)
        {
            Grid newGrid(grid.Width, grid.Height, grid.Blocks.get(), grid.Solution.Append(i));
            newGrid.RemoveGroup(groups[i]);

            Score newScore = scoring->RemoveGroup(score, grid, groups[i], newGrid, minGroupSize);

            if (newGrid.IsEmpty() || std::isnan(newScore.Objective))
                CheckSolution(newScore, newGrid, stop);
            else
                if (!dontAddToDB)
                {
                    std::unique_lock lock(mutex);
                    GridHashSet& hashSet = newGrids[newScore];
                    lock.unlock();

                    auto [it, inserted] = hashSet.insert(CompactGrid(std::move(newGrid)));
                    if (inserted)
                        c++;
                }
        }

        return c;
    }

    void Solver::CheckSolution(Score score, const Grid& grid, bool& stop)
    {
        if (!stop && score.Value < bestScore)
        {
            std::scoped_lock lock(mutex);

            if (!stop && score.Value < bestScore)
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

    void Solver::TrimDatabase()
    {
        if (MaxDBSize && multiplier > 1)
        {
            unsigned int reducedDBSize = std::ceil(*MaxDBSize / multiplier * TrimmingSafetyFactor);

            if (dbSize > reducedDBSize)
            {
                unsigned int accumulatedSize = 0;

                auto it = grids.begin();
                for (; ; it++)
                {
                    accumulatedSize += it->second.size();
                    if (accumulatedSize >= reducedDBSize)
                        break;
                }

                GridHashSet& hashSet = it->second;
                auto it2 = hashSet.begin();
                std::advance(it2, accumulatedSize - reducedDBSize);
                hashSet.erase(hashSet.begin(), it2);

                it++;
                grids.erase(it, grids.end());

                dbSize = reducedDBSize;
            }
        }
    }
}