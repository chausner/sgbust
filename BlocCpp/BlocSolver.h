#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "parallel_hashmap/phmap.h"
#include <mutex>
#include "BlockGrid.h"

struct BlockGridHash
{
	size_t operator()(const BlockGrid& blockGrid) const
	{
		if constexpr (sizeof(size_t) == 4)
		{
			size_t hash = 2166136261;

			for (int i = 0; i < blockGrid.Width * blockGrid.Height; i++)
				hash = (hash ^ (int)blockGrid.Blocks[i]) * 16777619;

			return hash;
		}
		else if constexpr (sizeof(size_t) == 8)
		{
			size_t hash = 14695981039346656037;

			for (int i = 0; i < blockGrid.Width * blockGrid.Height; i++)
				hash = (hash ^ (int)blockGrid.Blocks[i]) * 1099511628211;

			return hash;
		}
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
	unsigned int smallestGroupSize;

	std::vector<BlockGridHashSet*> blockGrids;

	std::unique_ptr<BlockGrid> bestGrid;
	unsigned int bestScore;

	unsigned int depth;
	unsigned int worstNumberOfBlocks;
	unsigned int dbSize;

public:
	void Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, std::optional<unsigned int> maxDBSize, std::optional<unsigned int> maxDepth, bool save, bool dontAddToDBLastDepth);

private:
	void SolveDepth(std::optional<unsigned int> maxDBSize, bool& stop, bool dontAddToDB = false);
	unsigned int SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks,
		std::vector<BlockGridHashSet*>& newBlockGrids,
		std::atomic_uint& newWorstNumberOfBlocks, bool& stop, bool dontAddToDB = false);
};