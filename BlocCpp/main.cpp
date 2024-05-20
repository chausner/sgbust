#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <random>
#include <unordered_set>
#include "bloc/Grid.h"
#include "bloc/Solver.h"
#include "utils.h"
#include "CLI/CLI.hpp"
#include "mimalloc-new-delete.h"

struct SolveCLIOptions
{
    std::string GridFile;
	std::string SolutionPrefix;
	std::optional<unsigned int> MaxDBSize = std::nullopt;
	std::optional<unsigned int> MaxDepth = std::nullopt;
	bool DontAddToDBLastDepth = false;
	bool TrimDB = true;
	double TrimmingSafetyFactor = 1.25;
	bool Quiet = false;
};

struct GenerateCLIOptions
{
	std::string GridFile;
	std::optional<unsigned long long> Seed;
	unsigned char Width;
	unsigned char Height;
	unsigned int NumColors;
	unsigned int SmallestGroupSize;
	bool Quiet = false;
};

struct ShowCLIOptions
{
	std::string GridFile;
	std::string Solution;
};

static void RunSolveCommand(const SolveCLIOptions &cliOptions)
{
#ifdef _WIN32
	EnableVTMode();
#endif

	unsigned int smallestGroupSize;

	std::ifstream file(cliOptions.GridFile, std::ios_base::binary);
	bloc::Grid grid(file, smallestGroupSize);
	file.close();

	if (!cliOptions.Quiet)
		grid.Print();

	bloc::Solver solver;

	solver.MaxDBSize = cliOptions.MaxDBSize;
	solver.MaxDepth = cliOptions.MaxDepth;
	solver.DontAddToDBLastDepth = cliOptions.DontAddToDBLastDepth;
	solver.TrimDB = cliOptions.TrimDB;
	solver.TrimmingSafetyFactor = cliOptions.TrimmingSafetyFactor;
	solver.Quiet = cliOptions.Quiet;

	auto startTime = std::chrono::steady_clock::now();

	solver.Solve(grid, smallestGroupSize, bloc::Solution(cliOptions.SolutionPrefix));

	auto endTime = std::chrono::steady_clock::now();

	auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	if (!cliOptions.Quiet)
		std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
}

static void RunGenerateCommand(const GenerateCLIOptions &cliOptions)
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

	bloc::Grid grid = bloc::Grid::GenerateRandom(cliOptions.Width, cliOptions.Height, cliOptions.NumColors, mt);

	std::ofstream file(cliOptions.GridFile, std::ios_base::binary);
	grid.Save(file, cliOptions.SmallestGroupSize);
	file.close();

	if (!cliOptions.Quiet)
	{
#ifdef _WIN32
		EnableVTMode();
#endif

		grid.Print();
	}
}

static void RunShowCommand(const ShowCLIOptions &cliOptions)
{
#ifdef _WIN32
	EnableVTMode();
#endif

	unsigned int smallestGroupSize;

	std::ifstream file(cliOptions.GridFile, std::ios_base::binary);
	bloc::Grid grid(file, smallestGroupSize);
	file.close();

	std::unordered_set<bloc::Block> colors(grid.BlocksBegin(), grid.BlocksEnd());
	colors.erase(bloc::Block::None);
	int numColors = colors.size();

	std::cout << "Size: " << static_cast<int>(grid.Width) << " x " << static_cast<int>(grid.Height) << std::endl;
	std::cout << "Number of colors: " << numColors << std::endl;
	std::cout << "Smallest group size: " << smallestGroupSize << std::endl;
	std::cout << std::endl;
	grid.Print();

	if (!cliOptions.Solution.empty())
	{
		auto pluralS = [](int x) { return x == 1 ? "" : "s"; };

		bloc::Solution solution(cliOptions.Solution);
		std::cout << std::endl;
		std::cout << "Solution: " << solution.AsString() << " (" << solution.GetLength() << " step" << pluralS(solution.GetLength()) << ")" << std::endl;

		bloc::Grid bg = grid;

		for (unsigned int i = 0; i < solution.GetLength(); i++)
		{
			unsigned char step = solution[i];
			std::vector<std::vector<bloc::Position>> groups;
			bg.GetGroups(groups, smallestGroupSize);
			if (step >= groups.size())
				throw std::invalid_argument("Solution string is not valid for this block grid");
			bg.RemoveGroup(groups[step]);
			std::cout << (i + 1) << ". " << groups[step].size() << " block" << pluralS(groups[step].size()) << std::endl;
			bg.Print();
		}
	}
}

int main(int argc, const char* argv[])
{
	try
	{
		CLI::App app("SameGame solver", "BlocCpp");

		SolveCLIOptions cliOptions;

		CLI::App* solveCommand = app.add_subcommand("solve", "Solve a block grid");
		solveCommand->add_option("grid-file", cliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
		solveCommand->add_option("--prefix", cliOptions.SolutionPrefix, "Solution prefix");
		solveCommand->add_option("-s,--max-db-size", cliOptions.MaxDBSize, "Maximum DB size");
		solveCommand->add_option("-d,--max-depth", cliOptions.MaxDepth, "Maximum search depth");
		solveCommand->add_flag("--dont-add-to-db-last-depth", cliOptions.DontAddToDBLastDepth, "Do not add last depth to database");
		solveCommand->add_flag("!--no-trim", cliOptions.TrimDB, "Disable DB trimming");
		solveCommand->add_option("--trimming-safety-factor", cliOptions.TrimmingSafetyFactor, "Trimming safety factor");
		solveCommand->add_flag("-q,--quiet", cliOptions.Quiet, "Quiet mode");
		solveCommand->callback([&]() { RunSolveCommand(cliOptions); });

		GenerateCLIOptions generateCliOptions;

		CLI::App* generateCommand = app.add_subcommand("generate", "Generate a random block grid and save it to a file");
		generateCommand->add_option("grid-file", generateCliOptions.GridFile, "Destination path for Bloc Grid File (.bgf)")->required();
		generateCommand->add_option("--seed", generateCliOptions.Seed, "Seed to use for randomization");
		generateCommand->add_option("--width", generateCliOptions.Width, "Number of columns in the grid")->check(CLI::Range(1, 255))->required();
		generateCommand->add_option("--height", generateCliOptions.Height, "Number of rows in the grid")->check(CLI::Range(1, 255))->required();
		generateCommand->add_option("--num-colors", generateCliOptions.NumColors, "Number of colors in the grid")->check(CLI::Range(1, 7))->required();
		generateCommand->add_option("--smallest-group-size", generateCliOptions.SmallestGroupSize, "Smallest group size")->check(CLI::Range(1, 255 * 255))->required();
		generateCommand->add_flag("-q,--quiet", generateCliOptions.Quiet, "Quiet mode");
		generateCommand->callback([&]() { RunGenerateCommand(generateCliOptions); });

		ShowCLIOptions showCliOptions;

		CLI::App* showCommand = app.add_subcommand("show", "Show a block grid");
		showCommand->add_option("grid-file", showCliOptions.GridFile, "Bloc Grid File (.bgf)")->required()->check(CLI::ExistingFile);
		showCommand->add_option("--solution", showCliOptions.Solution, "Solution steps to show");
		showCommand->callback([&]() { RunShowCommand(showCliOptions); });

		app.require_subcommand(1);

		CLI11_PARSE(app, argc, argv);

		return 0;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "An unknown exception occurred";
		return 1;
	}
}