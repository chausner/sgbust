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
#include "CompactBlockGrid.h"
#include "MemoryUsage.h"

std::optional<SolverResult> BlocSolver::Solve(const BlockGrid& blockGrid, unsigned int smallestGroupSize, const Solution& solutionPrefix)
{
	this->smallestGroupSize = smallestGroupSize;
	this->solutionPrefix = solutionPrefix;

	BlockGrid bg = blockGrid;

	if (!solutionPrefix.IsEmpty())
		bg.ApplySolution(solutionPrefix, smallestGroupSize);
	
	blockGrids.clear();
	blockGrids[Scoring(bg, smallestGroupSize)].insert(CompactBlockGrid(bg));

	solution = Solution();
	bestScore = std::numeric_limits<int>::max();
	dbSize = 1;
	multiplier = 0;

	bool stop = false;

	for (depth = 0; depth < MaxDepth || !MaxDepth; depth++)
	{
		if (!Quiet)
			PrintStats();

		if (TrimDB)
			TrimDatabase();

		SolveDepth(stop, DontAddToDBLastDepth && MaxDepth && depth == *MaxDepth - 1);

		if (stop)
			break;
	}

	if (bestScore != std::numeric_limits<int>::max())
		return SolverResult { bestScore, std::move(solution) };
	else
		return std::nullopt;
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
		", hash sets: " << std::setw(4) << blockGrids.size() <<
		", scores (min/avg/max): " << minScore << "/" << std::fixed << std::setprecision(1) << avgScore << "/" << maxScore;

	std::optional<std::size_t> memoryUsage = GetCurrentMemoryUsage();
	if (memoryUsage.has_value())
		std::cout << ", memory: " << (*memoryUsage / 1024 / 1024) << "MB";
		
	std::cout << std::endl;
}

void BlocSolver::SolveDepth(bool& stop, bool dontAddToDB)
{
	std::map<Scoring, BlockGridHashSet> newBlockGrids;

	std::atomic_uint blockGridsSolved = 0;
	std::atomic_uint newDBSize = 0;

	for (auto it = blockGrids.begin(); it != blockGrids.end(); it = blockGrids.erase(it))
	{
		auto& [scoring, hashSet] = *it;

		// std::for_each is a little bit faster here than hashSet.for_each but only MSVC supports parallel execution for it,
		// therefore we fall back to hashSet.for_each for other compilers
#ifdef _MSC_VER
		std::for_each(std::execution::par, hashSet.begin(), hashSet.end(), [&](const CompactBlockGrid& blockGrid) {
#else
		hashSet.for_each(std::execution::par, [&](const CompactBlockGrid& blockGrid) {
#endif
			if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
				return;

			newDBSize += SolveBlockGrid(blockGrid.Expand(), scoring, newBlockGrids, stop, dontAddToDB);

			blockGridsSolved++;

			// overall, deallocation is faster if we deallocate the data inside CompactBlockGrids here already
			const_cast<CompactBlockGrid&>(blockGrid) = CompactBlockGrid();
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
	static thread_local std::vector<std::vector<Position>> groups;
	blockGrid.GetGroups(groups, smallestGroupSize);

	if (groups.empty())
		CheckSolution(scoring, blockGrid, stop);		

	int c = 0;

	for (int i = 0; i < groups.size(); i++)
	{
		BlockGrid newBlockGrid(blockGrid.Width, blockGrid.Height, blockGrid.Blocks.get(), blockGrid.Solution.Append(i));
		newBlockGrid.RemoveGroup(groups[i]);

		Scoring newScoring = scoring.RemoveGroup(blockGrid, groups[i], newBlockGrid, smallestGroupSize);

		if (newBlockGrid.IsEmpty())
			CheckSolution(newScoring, newBlockGrid, stop);
		else
			if (!dontAddToDB)
			{
				std::unique_lock lock(mutex);
				BlockGridHashSet& hashSet = newBlockGrids[newScoring];
				lock.unlock();

				auto [it, inserted] = hashSet.insert(CompactBlockGrid(std::move(newBlockGrid)));
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
			solution = solutionPrefix.Append(blockGrid.Solution);

			if (!Quiet)
			{
				std::cout << "Better solution found (score: " << score
					<< ", blocks: " << blockGrid.GetNumberOfBlocks()
					<< ", steps: " << solution.GetLength() << "): " << solution.AsString()
					<< std::endl;

				blockGrid.Print();
			}

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