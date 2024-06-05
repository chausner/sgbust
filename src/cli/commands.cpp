#include "cli/commands.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
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

    sgbust::LeftoverPenaltyFunc leftoverPenalty;
    if (cliOptions.ScoringLeftoverPenalty.has_value())
        leftoverPenalty = std::bind_front(&sgbust::Polynom::Evaluate, cliOptions.ScoringLeftoverPenalty);
    else
        leftoverPenalty = nullptr;

    std::unique_ptr<sgbust::Scoring> scoring;

    switch (cliOptions.ScoringType)
    {
    case ScoringType::Greedy:
        if (!cliOptions.ScoringGroupScore.has_value())
            throw CLI::ExcludesError("--scoring-group-score must be specified for scoring type 'greedy'", CLI::ExitCodes::ExcludesError);
        scoring = std::make_unique<sgbust::GreedyScoring>(
            std::bind_front(&sgbust::Polynom::Evaluate, *cliOptions.ScoringGroupScore),
            cliOptions.ScoringClearanceBonus.value_or(0),
            std::move(leftoverPenalty)
        );
        break;
    case ScoringType::Potential:
        if (cliOptions.ScoringGroupScore.has_value())
            throw CLI::ExcludesError("--scoring-group-score cannot be specified for scoring type 'potential", CLI::ExitCodes::ExcludesError);
        /*if (cliOptions.ScoringClearanceBonus.has_value())
            throw CLI::ExcludesError("--scoring-clearance-bonus cannot be specified for scoring type 'potential'", CLI::ExitCodes::ExcludesError);
        if (cliOptions.ScoringLeftoverPenalty.has_value())
            throw CLI::ExcludesError("--scoring-leftover-penalty cannot be specified for scoring type 'potential'", CLI::ExitCodes::ExcludesError);*/
        scoring = std::make_unique<sgbust::PotentialScoring>(
            std::bind_front(&sgbust::Polynom::Evaluate, *cliOptions.ScoringGroupScore),
            cliOptions.ScoringClearanceBonus.value_or(0),
            std::move(leftoverPenalty)
        );
        break;
    case ScoringType::NumBlocksNotInGroups:
        if (cliOptions.ScoringGroupScore.has_value())
            throw CLI::ExcludesError("--scoring-group-score cannot be specified for scoring type 'num-blocks-not-in-groups'", CLI::ExitCodes::ExcludesError);
        if (cliOptions.ScoringClearanceBonus.has_value())
            throw CLI::ExcludesError("--scoring-clearance-bonus cannot be specified for scoring type 'num-blocks-not-in-groups'", CLI::ExitCodes::ExcludesError);
        if (cliOptions.ScoringLeftoverPenalty.has_value())
            throw CLI::ExcludesError("--scoring-leftover-penalty cannot be specified for scoring type 'num-blocks-not-in-groups'", CLI::ExitCodes::ExcludesError);
        scoring = std::make_unique<sgbust::NumBlocksNotInGroupsScoring>();
        break;
    }

    std::optional<sgbust::SolverResult> solverResult = solver.Solve(grid, minGroupSize, *scoring, sgbust::Solution(cliOptions.SolutionPrefix));

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
