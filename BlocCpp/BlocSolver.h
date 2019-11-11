#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include "parallel_hashmap/phmap.h"
#include "BlockGrid.h"
#include "xxh3.h"

template <>
class std::hash<BlockGrid>
{
public:
	std::size_t operator()(const BlockGrid& key) const
	{
		return XXH3_64bits(key.Blocks.get(), key.Width * key.Height * sizeof(BlockColor));
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

class BlocSolver
{
	unsigned int smallestGroupSize = 0;

	std::vector<BlockGridHashSet*> blockGrids;

	std::vector<unsigned char> solution;
	unsigned int bestScore = 0;

	unsigned int depth = 0;
	unsigned int worstNumberOfBlocks = 0;
	unsigned int dbSize = 0;

public:
	void Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, std::optional<unsigned int> maxDBSize, std::optional<unsigned int> maxDepth, bool save, bool dontAddToDBLastDepth);

private:
	void SolveDepth(std::optional<unsigned int> maxDBSize, bool& stop, bool dontAddToDB = false);
	unsigned int SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks,
		std::vector<BlockGridHashSet*>& newBlockGrids,
		std::atomic_uint& newWorstNumberOfBlocks, bool& stop, bool dontAddToDB = false);
};