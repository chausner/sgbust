#include "cli/commands.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <thread>
#include <unordered_set>

#include "CLI/CLI.hpp"
#include "cli/parser.h"
#include "core/Grid.h"
#include "core/Solver.h"

void RunCommand(const SolveCLIOptions& cliOptions)
{
    unsigned int minGroupSize;

    std::ifstream file(cliOptions.GridFile, std::ios_base::binary);
    sgbust::Grid grid(file, minGroupSize);
    file.close();

    if (!cliOptions.Quiet)
        grid.Print();

    sgbust::Solver solver;

    solver.MaxDBSize = cliOptions.MaxDBSize;
    solver.MaxDepth = cliOptions.MaxDepth;
    solver.DontAddToDBLastDepth = cliOptions.DontAddToDBLastDepth;
    solver.TrimDB = cliOptions.TrimDB;
    solver.TrimmingSafetyFactor = cliOptions.TrimmingSafetyFactor;
    solver.Quiet = cliOptions.Quiet;

    auto startTime = std::chrono::steady_clock::now();

    std::optional<sgbust::SolverResult> solverResult = solver.Solve(grid, minGroupSize, *cliOptions.ScoringOptions.Scoring, sgbust::Solution(cliOptions.SolutionPrefix));

    auto endTime = std::chrono::steady_clock::now();

    auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    if (!cliOptions.Quiet)
    {
        std::cout << std::endl;
        std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
        if (solverResult.has_value())
        {
            grid.ApplySolution(solverResult->BestSolution, minGroupSize);
            std::cout << "Best solution (score: " << solverResult->BestScore
                << ", blocks: " << grid.GetNumberOfBlocks()
                << ", steps: " << solverResult->BestSolution.GetLength() << "): " << solverResult->BestSolution.AsString()
                << std::endl;            
            grid.Print();
        }
        else
            std::cout << "No solution found." << std::endl;
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
        std::cout << std::endl;
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

    std::cout << "Size: " << static_cast<int>(grid.Width) << " x " << static_cast<int>(grid.Height) << std::endl;
    std::cout << "Number of colors: " << numColors << std::endl;
    std::cout << "Minimal group size: " << minGroupSize << std::endl;
    std::cout << std::endl;
    grid.Print();

    if (!cliOptions.Solution.empty())
    {
        auto pluralS = [](int x) { return x == 1 ? "" : "s"; };

        sgbust::Solution solution(cliOptions.Solution);
        std::cout << std::endl;
        std::cout << "Solution: " << solution.AsString() << " (" << solution.GetLength() << " step" << pluralS(solution.GetLength()) << ")" << std::endl;

        sgbust::Grid bg = grid;

        for (unsigned int i = 0; i < solution.GetLength(); i++)
        {
            unsigned char step = solution[i];
            std::vector<std::vector<sgbust::Position>> groups;
            bg.GetGroups(groups, minGroupSize);
            if (step >= groups.size())
                throw std::invalid_argument("Solution string is not valid for this grid");
            bg.RemoveGroup(groups[step]);
            std::cout << (i + 1) << ". " << groups[step].size() << " block" << pluralS(groups[step].size()) << std::endl;
            bg.Print();
        }
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

    solver.MaxDBSize = cliOptions.MaxDBSize;
    solver.Quiet = true;

    std::cout << "Press Ctrl+C to cancel." << std::endl;

    unsigned long long gridsProcessed = 0;
    unsigned long long gridsSolved = 0;
    unsigned long long gridsCleared = 0;
    long long bestScoreSum = 0;
    unsigned long long blocksRemainingSum = 0;
    auto startTime = std::chrono::steady_clock::now();
    std::optional<std::chrono::steady_clock::time_point> lastStatsPrinted;
    constexpr std::chrono::duration<double> RefreshInterval = std::chrono::seconds(1);

    auto printStats = [&]() {
        if (lastStatsPrinted.has_value())
            std::cout << "\x1B[6F"; // move cursor to the beginning of the line six lines up

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        double elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count();
        auto zeroIfNaN = [](auto x) { return std::isnan(x) ? 0 : x; };
        double gridsPerSecond = zeroIfNaN(gridsProcessed / elapsedSeconds);
        double secondsPerGrid = zeroIfNaN(elapsedSeconds / gridsProcessed);
        double gridsSolvedPercent = zeroIfNaN(static_cast<double>(gridsSolved) / gridsProcessed * 100);
        double gridsClearedPercent = zeroIfNaN(static_cast<double>(gridsCleared) / gridsProcessed * 100);
        double averageScore = zeroIfNaN(static_cast<double>(bestScoreSum) / gridsSolved);
        double averageBlocksRemaining = zeroIfNaN(static_cast<double>(blocksRemainingSum) / gridsSolved);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Elapsed: " << elapsedSeconds << " seconds\x1B[K\n";
        std::cout << "Grids processed: " << gridsProcessed << "\x1B[K\n";
        std::cout << "Grids solved: " << gridsSolved << " (" << gridsSolvedPercent << "%)\x1B[K\n";
        std::cout << "Grids cleared: " << gridsCleared << " (" << gridsClearedPercent << "%)\x1B[K\n";
        std::cout << "Average score: " << averageScore << "\x1B[K\n";
        std::cout << "Average number of blocks remaining: " << averageBlocksRemaining << "\x1B[K\n";
        std::cout << "Speed: " << gridsPerSecond << " grids/second (" << secondsPerGrid << " seconds/grid)\x1B[K" << std::flush;
        };

    std::thread thread([&] {
        while (!cliOptions.NumGrids.has_value() || gridsProcessed < *cliOptions.NumGrids)
        {
            sgbust::Grid blockGrid = sgbust::Grid::GenerateRandom(cliOptions.Width, cliOptions.Height, cliOptions.NumColors, mt);

            auto result = solver.Solve(blockGrid, cliOptions.MinGroupSize, *cliOptions.ScoringOptions.Scoring);

            if (result.has_value())
            {
                gridsSolved++;
                bestScoreSum += result->BestScore;
                if (result->SolutionGrid.IsEmpty())
                    gridsCleared++;
                blocksRemainingSum += result->SolutionGrid.GetNumberOfBlocks();
            }

            gridsProcessed++;

            auto now = std::chrono::steady_clock::now();
            if (!lastStatsPrinted.has_value() || now - *lastStatsPrinted >= RefreshInterval)
            {
                printStats();
                lastStatsPrinted = now;
            }            
        }
        });

    thread.join();

    printStats();
    std::cout << std::endl;
}
