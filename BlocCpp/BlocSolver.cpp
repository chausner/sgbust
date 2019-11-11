#include <algorithm>
#include <atomic>
#include <execution>
#include <iostream>
#include "BlocSolver.h"

void BlocSolver::Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, std::optional<unsigned int> maxDBSize, std::optional<unsigned int> maxDepth, bool save, bool dontAddToDBLastDepth)
{
	this->smallestGroupSize = smallestGroupSize;

	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();
	
	blockGrids.clear();
	blockGrids[numberOfBlocks].insert(blockGrid);

	solution.clear();
	bestScore = std::numeric_limits<unsigned int>::max();

	depth = 0;
	dbSize = 1;

	bool stop = false;

	while (!stop && (!maxDepth || depth < maxDepth))
	{
		std::cout << "Depth: " << depth << ", Database size: " << dbSize << std::endl;

		SolveDepth(maxDBSize, stop, dontAddToDBLastDepth && maxDepth && depth == *maxDepth - 1);

		depth++;
	}

	if (bestScore != std::numeric_limits<unsigned int>::max())
		blockGrid.Solution = std::move(solution);
}

void BlocSolver::SolveDepth(std::optional<unsigned int> maxDBSize, bool& stop, bool dontAddToDB)
{
	std::map<unsigned int, BlockGridHashSet> newBlockGrids;

	std::atomic_uint blockGridsSolved = 0;
	std::atomic_uint newDBSize = 0;

	for (auto &[numberOfBlocks, hashSet] : blockGrids)
	{
		if (!stop && (!maxDBSize || newDBSize < maxDBSize))
			std::for_each(std::execution::par, hashSet.begin(), hashSet.end(), [&](const BlockGrid& blockGrid) {
				if (stop || (maxDBSize && newDBSize >= maxDBSize))
					return;

				newDBSize += SolveBlockGrid(blockGrid, numberOfBlocks, newBlockGrids, stop, dontAddToDB);

				blockGridsSolved++;
			});

		// ideally delete from blockGrids if possible
		hashSet.clear();
	}

	if (newDBSize == 0)
		stop = true;

	blockGrids = std::move(newBlockGrids);
	dbSize = newDBSize;
}

unsigned int BlocSolver::SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks, std::map<unsigned int, BlockGridHashSet>& newBlockGrids, bool& stop, bool dontAddToDB)
{
	std::vector<std::vector<Position>> groups = blockGrid.GetGroups(smallestGroupSize);

	if (groups.empty())
	{
		unsigned int score = (100 * numberOfBlocks) + blockGrid.Solution.size();

		if (score < bestScore)
		{
			bestScore = score;
			solution = blockGrid.Solution;

			std::cout << "Better solution found (" << score << "): " << blockGrid.GetSolutionAsString() << std::endl;
		}

		return 0;
	}

	int c = 0;

	for (int i = 0; i < groups.size(); i++)
	{
		BlockGrid bg = blockGrid;

		unsigned int newNumberOfBlocks = numberOfBlocks - groups[i].size();

		if (newNumberOfBlocks == 0)
		{
			bg.Solution.push_back(i);

			bestScore = bg.Solution.size();
			solution = bg.Solution;

			std::cout << "Best solution found (" << bestScore << "): " << bg.GetSolutionAsString() << std::endl;

			stop = true;

			return c + 1;
		}

		if (!dontAddToDB)
		{
			bg.RemoveGroup(groups[i]);

			bg.Solution.push_back(i);

			auto [it, inserted] = newBlockGrids[newNumberOfBlocks].insert(std::move(bg));
			if (inserted)
				c++;
		}
	}

	return c;
}