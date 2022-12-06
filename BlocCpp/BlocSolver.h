#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include "parallel_hashmap/phmap.h"
#include "BlockGrid.h"
#include "CompactBlockGrid.h"
#include "Scoring.h"
#include "wyhash.h"

template <>
class std::hash<CompactBlockGrid>
{
public:
	std::size_t operator()(const CompactBlockGrid& key) const
	{
		return wyhash(key.Data.get(), key.DataLength(), 0, _wyp);
	}
};

template <>
class std::equal_to<CompactBlockGrid>
{
public:
	constexpr bool operator()(const CompactBlockGrid& lhs, const CompactBlockGrid& rhs) const
	{
		return lhs.Width == rhs.Width &&
			lhs.Height == rhs.Height &&
			std::equal(lhs.Data.get(), lhs.Data.get() + lhs.DataLength(), rhs.Data.get());
	}
};

using BlockGridHashSet = phmap::parallel_flat_hash_set<CompactBlockGrid, std::hash<CompactBlockGrid>, std::equal_to<CompactBlockGrid>, std::allocator<CompactBlockGrid>, 6, std::mutex>;

struct SolverResult
{
	int BestScore;
	Solution BestSolution;
};

class BlocSolver
{
	unsigned int smallestGroupSize = 0;
	const Scoring* scoring = nullptr;
	unsigned int depth = 0;
	std::map<Score, BlockGridHashSet> blockGrids;
	Solution solutionPrefix;
	Solution solution;
	int bestScore = 0;
	unsigned int dbSize = 0;
	double multiplier = 0;
	std::mutex mutex;

	void SolveDepth(bool& stop, bool dontAddToDB);
	unsigned int SolveBlockGrid(const BlockGrid& blockGrid, Score score, std::map<Score, BlockGridHashSet>& newBlockGrids, bool& stop, bool dontAddToDB);
	void CheckSolution(Score score, const BlockGrid& blockGrid, bool& stop);
	void PrintStats() const;
	void TrimDatabase();

public:
	std::optional<unsigned int> MaxDBSize = std::nullopt;
	std::optional<unsigned int> MaxDepth = std::nullopt;
	bool DontAddToDBLastDepth = false;
	bool TrimDB = true;
	double TrimmingSafetyFactor = 1.25;
	bool Quiet = false;

	std::optional<SolverResult> Solve(const BlockGrid& blockGrid, unsigned int smallestGroupSize, const Scoring& scoring, const Solution& solutionPrefix = {});
};