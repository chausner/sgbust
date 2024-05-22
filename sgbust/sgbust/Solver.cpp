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
#include "Solver.h"
#include "CompactGrid.h"
#include "MemoryUsage.h"

namespace sgbust
{
	std::optional<SolverResult> Solver::Solve(const Grid& grid, unsigned int smallestGroupSize, const Solution& solutionPrefix)
	{
		this->smallestGroupSize = smallestGroupSize;
		this->solutionPrefix = solutionPrefix;

		Grid bg = grid;

		if (!solutionPrefix.IsEmpty())
			bg.ApplySolution(solutionPrefix, smallestGroupSize);

		grids.clear();
		grids[Scoring(bg, smallestGroupSize)].insert(CompactGrid(bg));

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
			return SolverResult{ bestScore, std::move(solution) };
		else
			return std::nullopt;
	}

	void Solver::PrintStats() const
	{
		int minScore = grids.begin()->first.Score;
		int maxScore = grids.rbegin()->first.Score;
		long long scoreSum = std::transform_reduce(grids.begin(), grids.end(), 0LL, std::plus<>(), [](auto& b) { return b.first.Score * b.second.size(); });
		double avgScore = static_cast<double>(scoreSum) / dbSize;
		std::cout <<
			"Depth: " << std::setw(3) << depth <<
			", grids: " << std::setw(9) << dbSize <<
			", hash sets: " << std::setw(4) << grids.size() <<
			", scores (min/avg/max): " << minScore << "/" << std::fixed << std::setprecision(1) << avgScore << "/" << maxScore;

		std::optional<std::size_t> memoryUsage = GetCurrentMemoryUsage();
		if (memoryUsage.has_value())
			std::cout << ", memory: " << (*memoryUsage / 1024 / 1024) << "MB";

		std::cout << std::endl;
	}

	void Solver::SolveDepth(bool& stop, bool dontAddToDB)
	{
		std::map<Scoring, GridHashSet> newGrids;

		std::atomic_uint gridsSolved = 0;
		std::atomic_uint newDBSize = 0;

		for (auto it = grids.begin(); it != grids.end(); it = grids.erase(it))
		{
			auto& [scoring, hashSet] = *it;

			// std::for_each is a little bit faster here than hashSet.for_each but only MSVC supports parallel execution for it,
			// therefore we fall back to hashSet.for_each for other compilers
#ifdef _MSC_VER
			std::for_each(std::execution::par, hashSet.begin(), hashSet.end(), [&](const CompactGrid& grid) {
#else
			hashSet.for_each(std::execution::par, [&](const CompactGrid& grid) {
#endif
				if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
					return;

				newDBSize += SolveGrid(grid.Expand(), scoring, newGrids, stop, dontAddToDB);

				gridsSolved++;

				// overall, deallocation is faster if we deallocate the data inside CompactGrids here already
				const_cast<CompactGrid&>(grid) = CompactGrid();
				});

			if (stop || (MaxDBSize && newDBSize >= MaxDBSize))
				break;
		}

		multiplier = static_cast<double>(newDBSize) / gridsSolved;

		if (newDBSize == 0)
			stop = true;

		grids = std::move(newGrids);
		dbSize = newDBSize;
	}

	unsigned int Solver::SolveGrid(const Grid & grid, Scoring scoring, std::map<Scoring, GridHashSet>&newGrids, bool& stop, bool dontAddToDB)
	{
		static thread_local std::vector<std::vector<Position>> groups;
		grid.GetGroups(groups, smallestGroupSize);

		if (groups.empty())
			CheckSolution(scoring, grid, stop);

		int c = 0;

		for (int i = 0; i < groups.size(); i++)
		{
			Grid newGrid(grid.Width, grid.Height, grid.Blocks.get(), grid.Solution.Append(i));
			newGrid.RemoveGroup(groups[i]);

			Scoring newScoring = scoring.RemoveGroup(grid, groups[i], newGrid, smallestGroupSize);

			if (newGrid.IsEmpty())
				CheckSolution(newScoring, newGrid, stop);
			else
				if (!dontAddToDB)
				{
					std::unique_lock lock(mutex);
					GridHashSet& hashSet = newGrids[newScoring];
					lock.unlock();

					auto [it, inserted] = hashSet.insert(CompactGrid(std::move(newGrid)));
					if (inserted)
						c++;
				}
		}

		return c;
	}

	void Solver::CheckSolution(Scoring scoring, const Grid & grid, bool& stop)
	{
		int score = scoring.Score;

		if (!stop && score < bestScore)
		{
			std::scoped_lock lock(mutex);

			if (!stop && score < bestScore)
			{
				bestScore = score;
				solution = solutionPrefix.Append(grid.Solution);

				if (!Quiet)
				{
					std::cout << "Better solution found (score: " << score
						<< ", blocks: " << grid.GetNumberOfBlocks()
						<< ", steps: " << solution.GetLength() << "): " << solution.AsString()
						<< std::endl;

					grid.Print();
				}

				if (scoring.IsPerfect())
					stop = true;
			}
		}
	}

	void Solver::TrimDatabase()
	{
		if (MaxDBSize && multiplier > 1)
		{
			unsigned int reducedDBSize = std::ceil(*MaxDBSize / multiplier * TrimmingSafetyFactor);

			if (dbSize > reducedDBSize)
			{
				unsigned int accumulatedSize = 0;

				auto it = grids.begin();
				for (; ; it++)
				{
					accumulatedSize += it->second.size();
					if (accumulatedSize > reducedDBSize)
						break;
				}

				GridHashSet& hashSet = it->second;
				auto it2 = hashSet.begin();
				std::advance(it2, accumulatedSize - reducedDBSize);
				hashSet.erase(hashSet.begin(), it2);

				it++;
				grids.erase(it, grids.end());
			}
		}
	}
}