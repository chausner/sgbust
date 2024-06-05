#include "cli/parser.h"

#include <unordered_map>

#include "CLI/CLI.hpp"

static const std::unordered_map<std::string, ScoringType> ScoringTypeStrings{
    { "greedy", ScoringType::Greedy },
    { "potential", ScoringType::Potential },
    { "num-blocks-not-in-groups", ScoringType::NumBlocksNotInGroups }
};

std::variant<CLIOptions, int> ParseArgs(int argc, const char* argv[])
{
    CLI::App app;

    app.name("sgbust");
    app.description("SameGame solver");

    std::optional<CLIOptions> cliOptions;

    SolveCLIOptions solveCliOptions;

    CLI::App* solveCommand = app.add_subcommand("solve", "Solve a grid");
    solveCommand->add_option("grid-file", solveCliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
    solveCommand->add_option("--scoring", solveCliOptions.ScoringType, "Type of scoring")->transform(CLI::CheckedTransformer(ScoringTypeStrings, CLI::ignore_case));
    solveCommand->add_option("--scoring-group-score", solveCliOptions.ScoringGroupScore, "Group score, as a function of the group size");
    solveCommand->add_option("--scoring-clearance-bonus", solveCliOptions.ScoringClearanceBonus, "Bonus for clearing a grid");
    solveCommand->add_option("--scoring-leftover-penalty", solveCliOptions.ScoringLeftoverPenalty, "Penalty when a grid is not cleared, as a function of the number of blocks left");
    solveCommand->add_option("--prefix", solveCliOptions.SolutionPrefix, "Solution prefix");
    solveCommand->add_option("-s,--max-db-size", solveCliOptions.MaxDBSize, "Maximum DB size");
    solveCommand->add_option("-d,--max-depth", solveCliOptions.MaxDepth, "Maximum search depth");
    solveCommand->add_flag("--dont-add-to-db-last-depth", solveCliOptions.DontAddToDBLastDepth, "Do not add last depth to database");
    solveCommand->add_flag("!--no-trim", solveCliOptions.TrimDB, "Disable DB trimming");
    solveCommand->add_option("--trimming-safety-factor", solveCliOptions.TrimmingSafetyFactor, "Trimming safety factor");
    solveCommand->add_flag("-q,--quiet", solveCliOptions.Quiet, "Quiet mode");
    solveCommand->callback([&] { cliOptions = solveCliOptions; });

    GenerateCLIOptions generateCliOptions;

    CLI::App* generateCommand = app.add_subcommand("generate", "Generate a random grid and save it to a file");
    generateCommand->add_option("grid-file", generateCliOptions.GridFile, "Destination path for Bloc Grid File (.bgf)")->required();
    generateCommand->add_option("--seed", generateCliOptions.Seed, "Seed to use for randomization");
    generateCommand->add_option("--width", generateCliOptions.Width, "Number of columns in the grid")->check(CLI::Range(1, 255))->required();
    generateCommand->add_option("--height", generateCliOptions.Height, "Number of rows in the grid")->check(CLI::Range(1, 255))->required();
    generateCommand->add_option("--num-colors", generateCliOptions.NumColors, "Number of colors in the grid")->check(CLI::Range(1, 7))->required();
    generateCommand->add_option("--min-group-size", generateCliOptions.MinGroupSize, "Minimal group size")->check(CLI::Range(1, 255 * 255))->required();
    generateCommand->add_flag("-q,--quiet", generateCliOptions.Quiet, "Quiet mode");
    generateCommand->callback([&] { cliOptions = generateCliOptions; });

    ShowCLIOptions showCliOptions;

    CLI::App* showCommand = app.add_subcommand("show", "Show a grid");
    showCommand->add_option("grid-file", showCliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
    showCommand->add_option("--solution", showCliOptions.Solution, "Solution steps to show");
    showCommand->callback([&] { cliOptions = showCliOptions; });

    CLI11_PARSE(app, argc, argv);

    if (!cliOptions.has_value())
        return app.exit(CLI::CallForHelp());

    return *cliOptions;
}