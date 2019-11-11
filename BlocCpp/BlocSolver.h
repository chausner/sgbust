#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <vector>
#include "parallel_hashmap/phmap.h"
#include "BlockGrid.h"
#include "xxh3.h"

struct BlockGridHash
{
	size_t operator()(const BlockGrid& blockGrid) const
	{
		return XXH3_64bits(blockGrid.Blocks.get(), blockGrid.Width * blockGrid.Height * sizeof(BlockColor));
	}
};

struct BlockGridEqualTo
{
	bool operator()(const BlockGrid& blockGrid1, const BlockGrid& blockGrid2) const
	{
		return blockGrid1.Equals(blockGrid2);
	}
};

using BlockGridHashSet = phmap::parallel_flat_hash_set<BlockGrid, BlockGridHash, BlockGridEqualTo, phmap::container_internal::Allocator<BlockGrid>, 5, std::mutex>; // phmap::NullMutex

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