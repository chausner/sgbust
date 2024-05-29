#include "cli/commands.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <optional>
#include <random>
#include <unordered_set>

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

	solver.Solve(grid, minGroupSize, sgbust::Solution(cliOptions.SolutionPrefix));

	auto endTime = std::chrono::steady_clock::now();

	auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	if (!cliOptions.Quiet)
		std::cout << "Done! - took " << elapsedMilliseconds << "ms" << std::endl;
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
		grid.Print();
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
