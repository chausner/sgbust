#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include "parallel_hashmap/phmap.h"
#include "BlockGrid.h"
#include "wyhash.h"

template <>
class std::hash<BlockGrid>
{
public:
	std::size_t operator()(const BlockGrid& key) const
	{
		return wyhash(key.Blocks.get(), key.Width * key.Height * sizeof(BlockColor), 0);
	}
};

template <>
class std::equal_to<BlockGrid>
{
public:
	constexpr bool operator()(const BlockGrid& lhs, const BlockGrid& rhs) const
	{
		return lhs.Width == rhs.Width &&
			lhs.Height == rhs.Height &&
			std::equal(lhs.BlocksBegin(), lhs.BlocksEnd(), rhs.BlocksBegin());
	}
};

using BlockGridHashSet = phmap::parallel_flat_hash_set<BlockGrid, std::hash<BlockGrid>, std::equal_to<BlockGrid>, std::allocator<BlockGrid>, 5, std::mutex>; // phmap::NullMutex

struct Scoring
{
	int Score;

	Scoring(const BlockGrid& blockGrid);
	Scoring(int score): Score(score) {}
	Scoring RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid);
	bool IsPerfect() const;

	bool operator<(const Scoring& other) const
	{
		return Score < other.Score;
	}
};

class BlocSolver
{
	unsigned int smallestGroupSize = 0;
	std::map<Scoring, BlockGridHashSet> blockGrids;
	std::vector<unsigned char> solution;
	int bestScore = 0;
	unsigned int dbSize = 0;
	std::mutex mutex;

	void SolveDepth(bool& stop, bool dontAddToDB);
	unsigned int SolveBlockGrid(const BlockGrid& blockGrid, Scoring scoring, std::map<Scoring, BlockGridHashSet>& newBlockGrids, bool& stop, bool dontAddToDB);
	void CheckSolution(Scoring scoring, const BlockGrid& blockGrid, bool& stop);

public:
	std::optional<unsigned int> MaxDBSize = std::nullopt;
	std::optional<unsigned int> MaxDepth = std::nullopt;
	bool DontAddToDBLastDepth = false;

	void Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize);
};