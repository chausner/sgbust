#include <atomic>
#include <cmath>
#include <execution>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>
#include "BlocSolver.h"

void BlocSolver::Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize)
{
	this->smallestGroupSize = smallestGroupSize;
	
	blockGrids.clear();
	blockGrids[Scoring(blockGrid, smallestGroupSize)].insert(blockGrid);

	solution = Solution();
	bestScore = std::numeric_limits<int>::max();

	dbSize = 1;

	bool stop = false;

	for (depth = 0; depth < MaxDepth || !MaxDepth; depth++)
	{
		PrintStats();

		SolveDepth(stop, DontAddToDBLastDepth && MaxDepth && depth == *MaxDepth - 1);

		if (stop)
			break;
	}

	if (bestScore != std::numeric_limits<int>::max())
		blockGrid.Solution = std::move(solution);
}

void BlocSolver::PrintStats() const
{	
	int minScore = blockGrids.begin()->first.Score;
	int maxScore = blockGrids.rbegin()->first.Score;
	long long scoreSum = std::transform_reduce(blockGrids.begin(), blockGrids.end(), 0LL, std::plus<>(), [](auto& b) { return b.first.Score * b.second.size(); });
	double avgScore = static_cast<double>(scoreSum) / dbSize;
	std::cout << 
		"Depth: " << std::setw(3) << depth << 
		", grids: " << std::setw(9) << dbSize << 
		", scores (min/avg/max): " << minScore << "/" << std::fixed << std::setprecision(1) << avgScore << "/" << maxScore << std::endl;
}


void BlocSolver::SolveDepth(bool& stop, bool dontAddToDB)
{
	if (TrimDB)
		TrimDatabase();

	std::map<Scoring, BlockGridHashSet> newBlockGrids;

	std::atomic_uint blockGridsSolved = 0;
	std::atomic_uint newDBSize = 0;

	std::vector<const BlockGrid*> hashSetItems;

	for (auto it = blockGrids.begin(); it != blockGrids.end(); it = blockGrids.erase(it))
	{
		auto& [scoring, hashSet] = *it;

		hashSetItems.clear();
		hashSetItems.reserve(hashSet.size());
		for (auto it = hashSet.begin(); it != hashSet.end(); it++)
			hashSetItems.push_back(&*it);

		std::for_each(std::execution::par, hashSetItems.begin(), hashSetItems.end(), [&](const BlockGrid* blockGrid) {
			if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
				return;

			newDBSize += SolveBlockGrid(*blockGrid, scoring, newBlockGrids, stop, dontAddToDB);

			blockGridsSolved++;
		});

		if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
			break;
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
		newBlockGrid.Solution = newBlockGrid.Solution.Append(i);

		Scoring newScoring = scoring.RemoveGroup(blockGrid, groups[i], newBlockGrid, smallestGroupSize);

		if (newBlockGrid.IsEmpty())
			CheckSolution(newScoring, newBlockGrid, stop);
		else
			if (!dontAddToDB)
			{
				std::unique_lock lock(mutex);
				BlockGridHashSet& hashSet = newBlockGrids[newScoring];
				lock.unlock();

				auto [it, inserted] = hashSet.insert(std::move(newBlockGrid));
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
		std::scoped_lock lock(mutex);

		if (!stop && score < bestScore)
		{
			bestScore = score;
			solution = blockGrid.Solution;

			std::cout << "Better solution found (score: " << score 
				<< ", blocks: " << blockGrid.GetNumberOfBlocks() 
				<< ", steps: " << solution.GetLength() << "): " << blockGrid.Solution.AsString()
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