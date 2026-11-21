#include "cli/commands.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <format>
#include <fstream>
#include <future>
#include <optional>
#include <print>
#include <random>
#include <stdexcept>
#include <unordered_set>

#include "CLI/CLI.hpp"
#include "cli/parser.h"
#include "core/Grid.h"
#include "core/Solver.h"

namespace sgbust::cli
{
    void RunCommand(const SolveCLIOptions& cliOptions)
    {
        unsigned int minGroupSize;

        std::ifstream file(cliOptions.GridFile, std::ios_base::binary);
        sgbust::Grid grid(file, minGroupSize);
        file.close();

        if (!cliOptions.Quiet)
            grid.Print();

        sgbust::Solver solver;

        solver.MaxBeamSize = cliOptions.MaxBeamSize;
        solver.MaxDepth = cliOptions.MaxDepth;
        solver.ClearingSolutionsOnly = cliOptions.ClearingSolutionsOnly;
        solver.TrimmingEnabled = cliOptions.TrimmingEnabled;
        solver.TrimmingSafetyFactor = cliOptions.TrimmingSafetyFactor;
        solver.BeamDumpPath = cliOptions.SaveBeam;
        solver.Quiet = cliOptions.Quiet;

        auto startTime = std::chrono::steady_clock::now();

        std::optional<sgbust::SolverResult> solverResult = solver.Solve(grid, minGroupSize, *cliOptions.ScoringOptions.Scoring, sgbust::Solution(cliOptions.SolutionPrefix));

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

        if (!cliOptions.Quiet)
        {
            std::println();
            std::println("Elapsed: {:%T}", elapsedMilliseconds);
            if (solverResult.has_value())
            {
                std::println("Best solution (score: {}, blocks: {}, steps: {}): {}",
                    solverResult->BestScore,
                    solverResult->SolutionGrid.GetNumberOfBlocks(),
                    solverResult->BestSolution.GetLength(),
                    solverResult->BestSolution.AsString());            
                solverResult->SolutionGrid.Print();
            }
            else
                std::println("No solution found.");
        }
    }

    void RunCommand(const GenerateCLIOptions& cliOptions)
    {
        std::mt19937_64 mt;

        if (!cliOptions.Seed.has_value())
        {
            std::random_device rand;
            std::array<std::seed_seq::result_type, decltype(mt)::state_size> seedData;
            std::generate(seedData.begin(), seedData.end(), std::ref(rand));
            std::seed_seq seed(seedData.begin(), seedData.end());
            mt.seed(seed);
        }
        else
            mt.seed(*cliOptions.Seed);

        sgbust::Grid grid = sgbust::Grid::GenerateRandom(cliOptions.Width, cliOptions.Height, cliOptions.NumColors, mt);

        std::ofstream file(cliOptions.GridFile, std::ios_base::binary);
        grid.Save(file, cliOptions.MinGroupSize);
        file.close();

        if (!cliOptions.Quiet)
        {
            grid.Print();
            std::println();
        }
    }

    void RunCommand(const ShowCLIOptions& cliOptions)
    {
        unsigned int minGroupSize;

        std::ifstream file(cliOptions.GridFile, std::ios_base::binary);
        sgbust::Grid grid(file, minGroupSize);
        file.close();

        std::unordered_set<sgbust::Block> colors(grid.BlocksBegin(), grid.BlocksEnd());
        colors.erase(sgbust::Block::None);
        int numColors = colors.size();

        std::println("Size: {} x {}", static_cast<int>(grid.Width), static_cast<int>(grid.Height));
        std::println("Number of colors: {}", numColors);
        std::println("Minimal group size: {}", minGroupSize);
        std::println();
        grid.Print();

        if (!cliOptions.Solution.empty())
        {
            auto pluralS = [](int x) { return x == 1 ? "" : "s"; };

            sgbust::Solution solution(cliOptions.Solution);
            std::println();
            std::println("Solution: {} ({} step{})", solution.AsString(), solution.GetLength(), pluralS(solution.GetLength()));

            sgbust::Grid bg = grid;

            for (unsigned int i = 0; i < solution.GetLength(); i++)
            {
                unsigned char step = solution[i];
                std::vector<sgbust::Group> groups;
                bg.GetGroups(groups, minGroupSize);
                if (step >= groups.size())
                    throw std::invalid_argument("Solution string is not valid for this grid");
                std::println("{}. {} block{}", i + 1, groups[step].size(), pluralS(groups[step].size()));
                bg.Print(&groups[step]);
                bg.RemoveGroup(groups[step]);
            }

            std::println("Final grid:");
            if (bg.IsEmpty())
                std::println("(empty)");
            else
                bg.Print();
        }
    }

    void RunCommand(const BenchmarkCLIOptions& cliOptions)
    {
        std::mt19937_64 mt;

        if (!cliOptions.Seed.has_value())
        {
            std::random_device rand;
            std::array<std::seed_seq::result_type, decltype(mt)::state_size> seedData;
            std::generate(seedData.begin(), seedData.end(), std::ref(rand));
            std::seed_seq seed(seedData.begin(), seedData.end());
            mt.seed(seed);
        }
        else
            mt.seed(*cliOptions.Seed);

        sgbust::Solver solver;

        solver.MaxBeamSize = cliOptions.MaxBeamSize;
        solver.Quiet = true;

        std::println("Press Ctrl+C to cancel.");

        unsigned long long gridsSolved = 0;
        unsigned long long gridsCleared = 0;
        long long bestScoreSum = 0;
        unsigned long long blocksRemainingSum = 0;
        auto startTime = std::chrono::steady_clock::now();
        std::optional<std::chrono::steady_clock::time_point> lastStatsPrinted;
        constexpr std::chrono::duration<double> RefreshInterval = std::chrono::seconds(1);

        auto printStats = [&]() {
            if (lastStatsPrinted.has_value())
                std::print("\x1B[5F"); // move cursor to the beginning of the line five lines up

            auto elapsed = std::chrono::steady_clock::now() - startTime;
            auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
            double elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count();
            auto zeroIfNaN = [](auto x) { return std::isnan(x) ? 0 : x; };
            double gridsPerSecond = zeroIfNaN(gridsSolved / elapsedSeconds);
            double secondsPerGrid = zeroIfNaN(elapsedSeconds / gridsSolved);
            double gridsClearedPercent = zeroIfNaN(static_cast<double>(gridsCleared) / gridsSolved * 100);
            double averageScore = zeroIfNaN(static_cast<double>(bestScoreSum) / gridsSolved);
            double averageBlocksRemaining = zeroIfNaN(static_cast<double>(blocksRemainingSum) / gridsSolved);

            std::println("Elapsed: {:%T}\x1B[K", elapsedMilliseconds);
            std::println("Grids solved: {}\x1B[K", gridsSolved);
            std::println("Grids cleared: {} ({:.2f}%)\x1B[K", gridsCleared, gridsClearedPercent);
            std::println("Average score: {:.2f}\x1B[K", averageScore);
            std::println("Average number of blocks remaining: {:.2f}\x1B[K", averageBlocksRemaining);
            std::print("Speed: {:.2f} grids/second ({:.2f} seconds/grid)\x1B[K", gridsPerSecond, secondsPerGrid);
            std::fflush(stdout);
            };

        std::future<void> process = std::async([&] {
            while (!cliOptions.NumGrids.has_value() || gridsSolved < *cliOptions.NumGrids)
            {
                sgbust::Grid blockGrid = sgbust::Grid::GenerateRandom(cliOptions.Width, cliOptions.Height, cliOptions.NumColors, mt);

                auto result = solver.Solve(blockGrid, cliOptions.MinGroupSize, *cliOptions.ScoringOptions.Scoring);

                if (!result.has_value())
                    throw std::logic_error("Solver::Solve unexpectedly returned std::nullopt");

                gridsSolved++;
                bestScoreSum += result->BestScore;
                if (result->SolutionGrid.IsEmpty())
                    gridsCleared++;
                blocksRemainingSum += result->SolutionGrid.GetNumberOfBlocks();

                auto now = std::chrono::steady_clock::now();
                if (!lastStatsPrinted.has_value() || now - *lastStatsPrinted >= RefreshInterval)
                {
                    printStats();
                    lastStatsPrinted = now;
                }            
            }
            });

        process.get();

        printStats();
        std::println();
    }
}