#include <iostream>
#include "BlocSolver.h"

void BlocSolver::Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, unsigned int maxDBSize, unsigned int maxDepth, bool save, bool dontAddToDBLastDepth)
{
	this->smallestGroupSize = smallestGroupSize;

	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();

	unsigned int hash = blockGrid.GetHash();

	worstNumberOfBlocks = numberOfBlocks;
	worstHash = hash;
	
	blockGrids.clear();
	blockGrids.resize(numberOfBlocks + 1);

	blockGrids[numberOfBlocks] = new std::vector<std::vector<BlockGrid>*>();	
	blockGrids[numberOfBlocks]->resize(hash + 1);

	(*blockGrids[numberOfBlocks])[hash] = new std::vector<BlockGrid>();
	(*blockGrids[numberOfBlocks])[hash]->push_back(blockGrid);

	bestGrid = nullptr;
	bestScore = UINT_MAX;

	depth = 0;
	dbSize = 1;

	bool stop = false;

	while (!stop && depth < maxDepth)
	{
		std::cout << "Depth: " << depth << ", Database size: " << dbSize << std::endl;

		SolveDepth(maxDBSize, stop, dontAddToDBLastDepth && depth == maxDepth - 1);          

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

	if (bestScore != UINT_MAX)
		blockGrid.Solution = bestGrid->Solution;
}

void BlocSolver::SolveDepth(unsigned int maxDBSize, bool& stop, bool dontAddToDB)
{
	std::vector<std::vector<std::vector<BlockGrid>*>*> newBlockGrids;

	newBlockGrids.resize(worstNumberOfBlocks + 1);

	unsigned int blockGridsSolved = 0;
	unsigned int newDBSize = 0;
	unsigned int newWorstNumberOfBlocks = 0;
	unsigned int newWorstHash = 0;

	for (int i = 0; i < blockGrids.size(); i++)
		if (blockGrids[i] != nullptr)
		{
			for (int j = 0; j < blockGrids[i]->size(); j++)
			{
				if ((*blockGrids[i])[j] != nullptr)
				{
					if (!stop && newDBSize < maxDBSize)
						for (int k = 0; k < (*blockGrids[i])[j]->size(); k++)
						{
							newDBSize += SolveBlockGrid((*(*blockGrids[i])[j])[k], i, j, newBlockGrids, newWorstNumberOfBlocks, newWorstHash, stop, dontAddToDB);

							blockGridsSolved++;

							if (stop || newDBSize >= maxDBSize)
								break;
						}

					delete (*blockGrids[i])[j];
				}
			}

			delete blockGrids[i];
		}

	if (newDBSize == 0)
		stop = true;

stop:
	blockGrids = newBlockGrids;
	dbSize = newDBSize;
	worstNumberOfBlocks = newWorstNumberOfBlocks;
	worstHash = newWorstHash;
}

unsigned int BlocSolver::SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks, unsigned int hash, 
	std::vector<std::vector<std::vector<BlockGrid>*>*>& newBlockGrids, 
	unsigned int& newWorstNumberOfBlocks, unsigned int& newWorstHash, bool& stop, bool dontAddToDB)
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

		BlockColor groupColor = bg.Blocks[groups[i][0].Y * bg.Width + groups[i][0].X];

		int newHash = hash - (HASH_FACTORS[(int)groupColor] * groups[i].size());

		if (newHash > newWorstHash)
			newWorstHash = newHash;

		if (newNumberOfBlocks == 0)
		{
			bg.Solution.push_back(i);

			bestScore = bg.Solution.size();
			bestGrid.reset(new BlockGrid(bg));

			std::cout << "Best solution found (" << bestScore << "): " << bg.GetSolutionAsString() << std::endl;

			stop = true;

			return c + 1;
		}

		bg.RemoveGroup(groups[i]);

		if (!dontAddToDB)
			if (!IsInDatabase(bg, newNumberOfBlocks, newHash, newBlockGrids))
			{
				bg.Solution.push_back(i);
				(*newBlockGrids[newNumberOfBlocks])[newHash]->push_back(bg);

				c++;
			}
	}

	return c;
}

bool BlocSolver::IsInDatabase(const BlockGrid& blockGrid, unsigned int numberOfBlocks, unsigned int hash, 
	std::vector<std::vector<std::vector<BlockGrid>*>*>& newBlockGrids) const
{
	if (newBlockGrids[numberOfBlocks] == nullptr)
	{
		newBlockGrids[numberOfBlocks] = new std::vector<std::vector<BlockGrid>*>();
		newBlockGrids[numberOfBlocks]->resize(worstHash + 1);
	}

	if ((*newBlockGrids[numberOfBlocks])[hash] == nullptr)
		(*newBlockGrids[numberOfBlocks])[hash] = new std::vector<BlockGrid>();
	else
	{
		std::vector<BlockGrid>& bgs = *(*newBlockGrids[numberOfBlocks])[hash];

		for (int i = 0; i < bgs.size(); i++)
			if (bgs[i].Equals(blockGrid))
				return true;
	}

	return false;
}