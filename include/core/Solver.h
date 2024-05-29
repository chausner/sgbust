#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

#include "core/CompactGrid.h"
#include "core/Grid.h"
#include "core/Scoring.h"
#include "mimalloc.h"
#include "parallel_hashmap/phmap.h"
#include "wyhash.h"

template <>
class std::hash<sgbust::CompactGrid>
{
public:
	std::size_t operator()(const sgbust::CompactGrid& key) const
	{
		return wyhash(key.Data.get(), key.DataLength(), 0, _wyp);
	}
};

template <>
class std::equal_to<sgbust::CompactGrid>
{
public:
	constexpr bool operator()(const sgbust::CompactGrid& lhs, const sgbust::CompactGrid& rhs) const
	{
		return lhs.Width == rhs.Width &&
			lhs.Height == rhs.Height &&
			std::equal(lhs.Data.get(), lhs.Data.get() + lhs.DataLength(), rhs.Data.get());
	}
};

namespace sgbust
{
	using GridHashSet = phmap::parallel_flat_hash_set<CompactGrid, std::hash<CompactGrid>, std::equal_to<CompactGrid>, mi_stl_allocator<CompactGrid>, 6, std::mutex>;

	struct SolverResult
	{
		int BestScore;
		Solution BestSolution;
	};

	class Solver
	{
		unsigned int minGroupSize = 0;
		unsigned int depth = 0;
		std::map<Scoring, GridHashSet> grids;
		Solution solutionPrefix;
		Solution solution;
		int bestScore = 0;
		unsigned int dbSize = 0;
		double multiplier = 0;
		std::mutex mutex;

		void SolveDepth(bool& stop, bool dontAddToDB);
		unsigned int SolveGrid(const Grid& grid, Scoring scoring, std::map<Scoring, GridHashSet>& newGrids, bool& stop, bool dontAddToDB);
		void CheckSolution(Scoring scoring, const Grid& grid, bool& stop);
		void PrintStats() const;
		void TrimDatabase();

	public:
		std::optional<unsigned int> MaxDBSize = std::nullopt;
		std::optional<unsigned int> MaxDepth = std::nullopt;
		bool DontAddToDBLastDepth = false;
		bool TrimDB = true;
		double TrimmingSafetyFactor = 1.25;
		bool Quiet = false;

		std::optional<SolverResult> Solve(const Grid& grid, unsigned int minGroupSize, const Solution& solutionPrefix = {});
	};
}