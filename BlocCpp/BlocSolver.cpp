#include <algorithm>
#include <atomic>
#include <execution>
#include <iostream>
#include <iterator>
#include "BlocSolver.h"

Scoring::Scoring(const BlockGrid& blockGrid) : Score(blockGrid.GetNumberOfBlocks())
{
}

Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid)
{
	return Score - group.size();

	/*unsigned int numberOfBlocks = newBlockGrid.GetNumberOfBlocks();
	auto groups = newBlockGrid.GetGroups(3);

	unsigned int numberOfBlocksInGroups = 0;
	for (auto& group : groups)
		numberOfBlocksInGroups += group.size();

	return numberOfBlocks - numberOfBlocksInGroups;*/

	//return Score - group.size() * (group.size() - 1);

	//return Score - ((group.size() - 2) * (group.size() - 2) + group.size());
}

bool Scoring::IsPerfect() const
{
	return Score == 0;
}

void BlocSolver::Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize)
{
	this->smallestGroupSize = smallestGroupSize;
	
	blockGrids.clear();
	blockGrids[Scoring(blockGrid)].insert(blockGrid);

	solution.clear();
	bestScore = std::numeric_limits<int>::max();

	dbSize = 1;

	bool stop = false;

	for (int depth = 0; depth < MaxDepth || !MaxDepth; depth++)
	{
		std::cout << "Depth: " << depth << ", Database size: " << dbSize << std::endl;

		SolveDepth(stop, DontAddToDBLastDepth && MaxDepth && depth == *MaxDepth - 1);

		if (stop)
			break;
	}

	if (bestScore != std::numeric_limits<int>::max())
		blockGrid.Solution = std::move(solution);
}

void BlocSolver::SolveDepth(bool& stop, bool dontAddToDB)
{
	if (TrimDB)
	    TrimDatabase();

	std::map<Scoring, BlockGridHashSet> newBlockGrids;

	std::atomic_uint blockGridsSolved = 0;
	std::atomic_uint newDBSize = 0;

	for (auto it = blockGrids.begin(); it != blockGrids.end(); it = blockGrids.erase(it))
	{
		auto& [scoring, hashSet] = *it;

		if (!stop && (!MaxDBSize || newDBSize < MaxDBSize))
			std::for_each(std::execution::par, hashSet.begin(), hashSet.end(), [&](const BlockGrid& blockGrid) {
				if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
					return;

				newDBSize += SolveBlockGrid(blockGrid, scoring, newBlockGrids, stop, dontAddToDB);

				blockGridsSolved++;
			});
	}

	multiplier = static_cast<double>(newDBSize) / blockGridsSolved;

	if (newDBSize == 0)
		stop = true;

	blockGrids = std::move(newBlockGrids);
	dbSize = newDBSize;
}

unsigned int BlocSolver::SolveBlockGrid(const BlockGrid& blockGrid, Scoring scoring, std::map<Scoring, BlockGridHashSet>& newBlockGrids, bool& stop, bool dontAddToDB)
{
	std::vector<std::vector<Position>> groups = blockGrid.GetGroups(smallestGroupSize);

	if (groups.empty())
		CheckSolution(scoring, blockGrid, stop);		

	int c = 0;

	for (int i = 0; i < groups.size(); i++)
	{
		BlockGrid newBlockGrid = blockGrid;

		newBlockGrid.RemoveGroup(groups[i]);
		newBlockGrid.Solution.push_back(i);

		Scoring newScoring = scoring.RemoveGroup(blockGrid, groups[i], newBlockGrid);

		if (newBlockGrid.IsEmpty())
			CheckSolution(newScoring, newBlockGrid, stop);
		else
			if (!dontAddToDB)
			{
				auto [it, inserted] = newBlockGrids[newScoring].insert(std::move(newBlockGrid));
				if (inserted)
					c++;
			}
	}

	return c;
}

void BlocSolver::CheckSolution(Scoring scoring, const BlockGrid& blockGrid, bool& stop)
{
	int score = scoring.Score;

	if (!stop && score < bestScore)
	{
		std::lock_guard lock(mutex);

		if (!stop && score < bestScore)
		{
			bestScore = score;
			solution = blockGrid.Solution;

			std::cout << "Better solution found (score: " << score 
				<< ", blocks: " << blockGrid.GetNumberOfBlocks() 
				<< ", steps: " << solution.size() << "): " << blockGrid.GetSolutionAsString() 
				<< std::endl;

			blockGrid.Print();

			if (scoring.IsPerfect())
				stop = true;
		}
	}
}

void BlocSolver::TrimDatabase()
{
	if (MaxDBSize && multiplier > 1)
	{
		unsigned int reducedDBSize = std::ceil(*MaxDBSize / multiplier * TrimmingSafetyFactor);

		if (dbSize > reducedDBSize)
		{
			unsigned int accumulatedSize = 0;

			auto it = blockGrids.begin();
			for (; ; it++)
			{
				accumulatedSize += it->second.size();
				if (accumulatedSize > reducedDBSize)
					break;
			}

			BlockGridHashSet& hashSet = it->second;
			auto it2 = hashSet.begin();
			std::advance(it2, accumulatedSize - reducedDBSize);
			hashSet.erase(hashSet.begin(), it2);

			it++;
			blockGrids.erase(it, blockGrids.end());
		}
	}
}