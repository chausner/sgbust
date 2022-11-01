#include <iostream>
#include <chrono>
#include <optional>
#include "BlockGrid.h"
#include "BlocSolver.h"
#include "utils.h"
#include "CLI/CLI.hpp"

struct CLIOptions
{
    std::string GridFile;
	std::optional<unsigned int> MaxDBSize = std::nullopt;
	std::optional<unsigned int> MaxDepth = std::nullopt;
	bool DontAddToDBLastDepth = false;
	bool TrimDB = true;
	double TrimmingSafetyFactor = 1.25;
	bool Quiet = false;
};

static void run(const CLIOptions &cliOptions)
{
#ifdef _WIN32
	EnableVTMode();
#endif

	unsigned int smallestGroupSize;

	BlockGrid blockGrid(cliOptions.GridFile, smallestGroupSize);

	if (!cliOptions.Quiet)
		blockGrid.Print();

	BlocSolver solver;

	solver.MaxDBSize = cliOptions.MaxDBSize;
	solver.MaxDepth = cliOptions.MaxDepth;
	solver.DontAddToDBLastDepth = cliOptions.DontAddToDBLastDepth;
	solver.TrimDB = cliOptions.TrimDB;
	solver.TrimmingSafetyFactor = cliOptions.TrimmingSafetyFactor;
	solver.Quiet = cliOptions.Quiet;

	auto startTime = std::chrono::steady_clock::now();

	solver.Solve(blockGrid, smallestGroupSize);

	auto endTime = std::chrono::steady_clock::now();

	auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	if (!cliOptions.Quiet)
		std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
}

int main(int argc, const char* argv[])
{
	CLI::App app("SameGame solver", "BlocCpp");

	CLIOptions cliOptions;

    app.add_option("grid-file", cliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
	app.add_option("-s,--max-db-size", cliOptions.MaxDBSize, "Maximum DB size");
	app.add_option("-d,--max-depth", cliOptions.MaxDepth, "Maximum search depth");
	app.add_flag("--dont-add-to-db-last-depth", cliOptions.DontAddToDBLastDepth, "Do not add last depth to database");
	app.add_flag("!--no-trim", cliOptions.TrimDB, "Disable DB trimming");
	app.add_option("--trimming-safety-factor", cliOptions.TrimmingSafetyFactor, "Trimming safety factor");
	app.add_flag("-q,--quiet", cliOptions.Quiet, "Quiet mode");

    CLI11_PARSE(app, argc, argv);

	run(cliOptions);

	return 0;
}