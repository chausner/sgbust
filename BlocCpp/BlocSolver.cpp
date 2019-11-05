#include <iostream>
#include "BlocSolver.h"

void BlocSolver::Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, std::optional<unsigned int> maxDBSize, std::optional<unsigned int> maxDepth, bool save, bool dontAddToDBLastDepth)
{
	this->smallestGroupSize = smallestGroupSize;

	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();

	worstNumberOfBlocks = numberOfBlocks;
	
	blockGrids.clear();
	blockGrids.resize(numberOfBlocks + 1);

	blockGrids[numberOfBlocks] = new std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>();	
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
	std::vector<std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>*> newBlockGrids;

	newBlockGrids.resize(worstNumberOfBlocks + 1);

	unsigned int blockGridsSolved = 0;
	unsigned int newDBSize = 0;
	unsigned int newWorstNumberOfBlocks = 0;

	for (int i = 0; i < blockGrids.size(); i++)
		if (blockGrids[i] != nullptr)
		{
			//std::cout << "Size " << i << ": " << blockGrids[i]->size() << std::endl;
			if (!stop && (!maxDBSize || newDBSize < maxDBSize))
				for (const BlockGrid& blockGrid : *(blockGrids[i]))
				{
					newDBSize += SolveBlockGrid(blockGrid, i, newBlockGrids, newWorstNumberOfBlocks, stop, dontAddToDB);

					blockGridsSolved++;

					if (stop || (maxDBSize && newDBSize >= maxDBSize))
						break;
				}

			delete blockGrids[i];
		}

	if (newDBSize == 0)
		stop = true;

stop:
	blockGrids = std::move(newBlockGrids);
	dbSize = newDBSize;
	worstNumberOfBlocks = newWorstNumberOfBlocks;
}

unsigned int BlocSolver::SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks, 
	std::vector<std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>*>& newBlockGrids, 
	unsigned int& newWorstNumberOfBlocks, bool& stop, bool dontAddToDB)
{
	std::vector<std::vector<Position>> groups = blockGrid.GetGroups(smallestGroupSize);

	if (groups.empty())
	{
		int score = (100 * numberOfBlocks) + blockGrid.Solution.size();

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

		int newNumberOfBlocks = numberOfBlocks - groups[i].size();

		if (newNumberOfBlocks > newWorstNumberOfBlocks)
			newWorstNumberOfBlocks = newNumberOfBlocks;

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

			if (!IsInDatabase(bg, newNumberOfBlocks, newBlockGrids))
			{
				bg.Solution.push_back(i);
				newBlockGrids[newNumberOfBlocks]->insert(std::move(bg));

				c++;
			}
		}
	}

	return c;
}

bool BlocSolver::IsInDatabase(const BlockGrid& blockGrid, unsigned int numberOfBlocks,
	std::vector<std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>*>& newBlockGrids) const
{
	if (newBlockGrids[numberOfBlocks] == nullptr)
	{
		newBlockGrids[numberOfBlocks] = new std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>();
	    return false;
	}

	return newBlockGrids[numberOfBlocks]->count(blockGrid) != 0;
}