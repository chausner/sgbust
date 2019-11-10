#include <algorithm>
#include <atomic>
#include <execution>
#include <iostream>
#include "BlocSolver.h"

void BlocSolver::Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, std::optional<unsigned int> maxDBSize, std::optional<unsigned int> maxDepth, bool save, bool dontAddToDBLastDepth)
{
	this->smallestGroupSize = smallestGroupSize;

	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();

	worstNumberOfBlocks = numberOfBlocks;
	
	blockGrids.clear();
	blockGrids.resize(numberOfBlocks + 1);

	blockGrids[numberOfBlocks] = new BlockGridHashSet();	
	blockGrids[numberOfBlocks]->insert(blockGrid);

	bestGrid = nullptr;
	bestScore = std::numeric_limits<unsigned int>::max();

	depth = 0;
	dbSize = 1;

	bool stop = false;

	while (!stop && (!maxDepth || depth < maxDepth))
	{
		std::cout << "Depth: " << depth << ", Database size: " << dbSize << std::endl;

		SolveDepth(maxDBSize, stop, dontAddToDBLastDepth && maxDepth && depth == *maxDepth - 1);

		depth++;

		//if (save)
		//	Save(string.Format(@"E:\Users\Christoph\Documents\Visual Studio 2015\Projects\Bloc\depth{ 0 }.dat", depth));
	}

	//for (auto a : blockGrids)
	//	if (a != nullptr)
	//		for (auto b : *a)
	//			if (b != nullptr)
	//				for (auto c : *b)
	//					std::cout << c.GetSolutionAsString() << "    " << c.GetNumberOfBlocks() << std::endl;

	if (bestScore != std::numeric_limits<unsigned int>::max())
		blockGrid.Solution = bestGrid->Solution;
}

void BlocSolver::SolveDepth(std::optional<unsigned int> maxDBSize, bool& stop, bool dontAddToDB)
{
	std::vector<BlockGridHashSet*> newBlockGrids;

	newBlockGrids.resize(worstNumberOfBlocks + 1);

	std::atomic_uint blockGridsSolved = 0;
	std::atomic_uint newDBSize = 0;
	std::atomic_uint newWorstNumberOfBlocks = 0;

	for (int i = 0; i < blockGrids.size(); i++)
		if (blockGrids[i] != nullptr)
		{
			//std::cout << "Size " << i << ": " << blockGrids[i]->size() << std::endl;
			if (!stop && (!maxDBSize || newDBSize < maxDBSize))
				std::for_each(std::execution::par, blockGrids[i]->begin(), blockGrids[i]->end(), [&](const BlockGrid& blockGrid) {
					if (stop || (maxDBSize && newDBSize >= maxDBSize))
						return;

					newDBSize += SolveBlockGrid(blockGrid, i, newBlockGrids, newWorstNumberOfBlocks, stop, dontAddToDB);

					blockGridsSolved++;
				});

			delete blockGrids[i];
		}

	if (newDBSize == 0)
		stop = true;

	blockGrids = std::move(newBlockGrids);
	dbSize = newDBSize;
	worstNumberOfBlocks = newWorstNumberOfBlocks;
}

template <typename T>
void atomicMax(std::atomic<T> &a, T b)
{
	bool success;
	T oldValue = a;
	do
	{
		if (b > oldValue)
			success = a.compare_exchange_weak(oldValue, b);
		else
			break;
	} while (!success);
}

unsigned int BlocSolver::SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks, std::vector<BlockGridHashSet*>& newBlockGrids, std::atomic_uint& newWorstNumberOfBlocks, bool& stop, bool dontAddToDB)
{
	std::vector<std::vector<Position>> groups = blockGrid.GetGroups(smallestGroupSize);

	if (groups.empty())
	{
		unsigned int score = (100 * numberOfBlocks) + blockGrid.Solution.size();

		if (score < bestScore)
		{
			bestScore = score;
			bestGrid.reset(new BlockGrid(blockGrid));

			std::cout << "Better solution found (" << score << "): " << blockGrid.GetSolutionAsString() << std::endl;
		}

		return 0;
	}

	int c = 0;

	for (int i = 0; i < groups.size(); i++)
	{
		BlockGrid bg = blockGrid;

		unsigned int newNumberOfBlocks = numberOfBlocks - groups[i].size();

		atomicMax<unsigned int>(newWorstNumberOfBlocks, newNumberOfBlocks);

		if (newNumberOfBlocks == 0)
		{
			bg.Solution.push_back(i);

			bestScore = bg.Solution.size();
			bestGrid.reset(new BlockGrid(bg));

			std::cout << "Best solution found (" << bestScore << "): " << bg.GetSolutionAsString() << std::endl;

			stop = true;

			return c + 1;
		}

		if (!dontAddToDB)
		{
			bg.RemoveGroup(groups[i]);

			bg.Solution.push_back(i);

			if (newBlockGrids[newNumberOfBlocks] == nullptr)
				newBlockGrids[newNumberOfBlocks] = new BlockGridHashSet();

			auto [it, inserted] = newBlockGrids[newNumberOfBlocks]->insert(std::move(bg));
			if (inserted)
				c++;
		}
	}

	return c;
}