#include "cli/parser.h"

#include <functional>
#include <unordered_map>

#include "CLI/CLI.hpp"
#include "core/scorings/GreedyScoring.h"
#include "core/scorings/NumBlocksNotInGroupsScoring.h"
#include "core/scorings/PotentialScoring.h"

static const std::unordered_map<std::string, ScoringType> ScoringTypeStrings{
    { "greedy", ScoringType::Greedy },
    { "potential", ScoringType::Potential },
    { "num-blocks-not-in-groups", ScoringType::NumBlocksNotInGroups }
};

static void AddScoringOptions(CLI::App* command, ScoringOptions& scoringOptions)
{
    command->add_option("--scoring", scoringOptions.ScoringType, "Type of scoring")->transform(CLI::CheckedTransformer(ScoringTypeStrings, CLI::ignore_case));
    command->add_option("--scoring-group-score", scoringOptions.ScoringGroupScore, "Group score, as a function of the group size");
    command->add_option("--scoring-clearance-bonus", scoringOptions.ScoringClearanceBonus, "Bonus for clearing a grid");
    command->add_option("--scoring-leftover-penalty", scoringOptions.ScoringLeftoverPenalty, "Penalty when a grid is not cleared, as a function of the number of blocks left");
}

static void ValidateAndSetScoring(ScoringOptions& scoringOptions)
{
    sgbust::LeftoverPenaltyFunc leftoverPenalty;
    if (scoringOptions.ScoringLeftoverPenalty.has_value())
        leftoverPenalty = std::bind_front(&sgbust::Polynom::Evaluate, *scoringOptions.ScoringLeftoverPenalty);
    else
        leftoverPenalty = nullptr;

    switch (scoringOptions.ScoringType)
    {
    case ScoringType::Greedy:
        if (!scoringOptions.ScoringGroupScore.has_value())
            throw CLI::ExcludesError("--scoring-group-score must be specified for scoring type 'greedy'", CLI::ExitCodes::ExcludesError);
        scoringOptions.Scoring = std::make_unique<sgbust::GreedyScoring>(
            std::bind_front(&sgbust::Polynom::Evaluate, *scoringOptions.ScoringGroupScore),
            scoringOptions.ScoringClearanceBonus.value_or(0),
            std::move(leftoverPenalty)
        );
        break;
    case ScoringType::Potential:
        if (!scoringOptions.ScoringGroupScore.has_value())
            throw CLI::ExcludesError("--scoring-group-score must be specified for scoring type 'potential'", CLI::ExitCodes::ExcludesError);
        scoringOptions.Scoring = std::make_unique<sgbust::PotentialScoring>(
            std::bind_front(&sgbust::Polynom::Evaluate, *scoringOptions.ScoringGroupScore),
            scoringOptions.ScoringClearanceBonus.value_or(0),
            std::move(leftoverPenalty)
        );
        break;
    case ScoringType::NumBlocksNotInGroups:
        if (scoringOptions.ScoringGroupScore.has_value())
            throw CLI::ExcludesError("--scoring-group-score cannot be specified for scoring type 'num-blocks-not-in-groups'", CLI::ExitCodes::ExcludesError);
        if (scoringOptions.ScoringClearanceBonus.has_value())
            throw CLI::ExcludesError("--scoring-clearance-bonus cannot be specified for scoring type 'num-blocks-not-in-groups'", CLI::ExitCodes::ExcludesError);
        if (scoringOptions.ScoringLeftoverPenalty.has_value())
            throw CLI::ExcludesError("--scoring-leftover-penalty cannot be specified for scoring type 'num-blocks-not-in-groups'", CLI::ExitCodes::ExcludesError);
        scoringOptions.Scoring = std::make_unique<sgbust::NumBlocksNotInGroupsScoring>();
        break;
    }
}

std::variant<CLIOptions, int> ParseArgs(int argc, const char* argv[])
{
    CLI::App app;

    app.name("sgbust");
    app.description("SameGame solver");

    std::optional<CLIOptions> cliOptions;

    SolveCLIOptions solveCliOptions;

    CLI::App* solveCommand = app.add_subcommand("solve", "Solve a grid");
    solveCommand->add_option("grid-file", solveCliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
    AddScoringOptions(solveCommand, solveCliOptions.ScoringOptions);
    solveCommand->add_option("--prefix", solveCliOptions.SolutionPrefix, "Solution prefix");
    solveCommand->add_option("-s,--max-beam-size", solveCliOptions.MaxBeamSize, "Maximum beam size");
    solveCommand->add_option("-d,--max-depth", solveCliOptions.MaxDepth, "Maximum search depth");
    solveCommand->add_flag("!--no-trim", solveCliOptions.TrimmingEnabled, "Disable beam trimming");
    solveCommand->add_option("--trimming-safety-factor", solveCliOptions.TrimmingSafetyFactor, "Trimming safety factor");
    solveCommand->add_flag("-q,--quiet", solveCliOptions.Quiet, "Quiet mode");
    solveCommand->callback([&] {
        ValidateAndSetScoring(solveCliOptions.ScoringOptions);

        cliOptions = std::move(solveCliOptions);
        });

    GenerateCLIOptions generateCliOptions;

    CLI::App* generateCommand = app.add_subcommand("generate", "Generate a random grid and save it to a file");
    generateCommand->add_option("grid-file", generateCliOptions.GridFile, "Destination path for Bloc Grid File (.bgf)")->required();
    generateCommand->add_option("--seed", generateCliOptions.Seed, "Seed to use for randomization");
    generateCommand->add_option("--width", generateCliOptions.Width, "Number of columns in the grid")->check(CLI::Range(1, 255))->required();
    generateCommand->add_option("--height", generateCliOptions.Height, "Number of rows in the grid")->check(CLI::Range(1, 255))->required();
    generateCommand->add_option("--num-colors", generateCliOptions.NumColors, "Number of colors in the grid")->check(CLI::Range(1, 7))->required();
    generateCommand->add_option("--min-group-size", generateCliOptions.MinGroupSize, "Minimal group size")->check(CLI::Range(1, 255 * 255))->required();
    generateCommand->add_flag("-q,--quiet", generateCliOptions.Quiet, "Quiet mode");
    generateCommand->callback([&] { cliOptions = std::move(generateCliOptions); });

    ShowCLIOptions showCliOptions;

    CLI::App* showCommand = app.add_subcommand("show", "Show a grid");
    showCommand->add_option("grid-file", showCliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
    showCommand->add_option("--solution", showCliOptions.Solution, "Solution steps to show");
    showCommand->callback([&] { cliOptions = std::move(showCliOptions); });

    BenchmarkCLIOptions benchmarkCliOptions;

    CLI::App* benchmarkCommand = app.add_subcommand("benchmark", "Run benchmarks");
    benchmarkCommand->add_option("--seed", benchmarkCliOptions.Seed, "Seed to use for randomization");
    benchmarkCommand->add_option("--width", benchmarkCliOptions.Width, "Number of columns in the grid")->check(CLI::Range(1, 255))->required();
    benchmarkCommand->add_option("--height", benchmarkCliOptions.Height, "Number of rows in the grid")->check(CLI::Range(1, 255))->required();
    benchmarkCommand->add_option("--num-colors", benchmarkCliOptions.NumColors, "Number of colors in the grid")->check(CLI::Range(1, 7))->required();
    benchmarkCommand->add_option("--min-group-size", benchmarkCliOptions.MinGroupSize, "Minimal group size")->check(CLI::Range(1, 255 * 255))->required();
    benchmarkCommand->add_option("--num-grids", benchmarkCliOptions.NumGrids, "Number of grids to generate and solve");
    AddScoringOptions(benchmarkCommand, benchmarkCliOptions.ScoringOptions);
    benchmarkCommand->add_option("--max-beam-size", benchmarkCliOptions.MaxBeamSize, "Maximum beam size");
    benchmarkCommand->callback([&]() { 
        ValidateAndSetScoring(benchmarkCliOptions.ScoringOptions);
        
        cliOptions = std::move(benchmarkCliOptions);
        });

    CLI11_PARSE(app, argc, argv);

    if (!cliOptions.has_value())
        return app.exit(CLI::CallForHelp());

    return std::move(*cliOptions);
}