#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include "BlockGrid.h"

struct BlockGridHash
{
	size_t operator()(const BlockGrid& blockGrid) const
	{
#if SIZE_MAX == UINT32_MAX
		size_t hash = 2166136261;

		for (int i = 0; i < blockGrid.Width * blockGrid.Height; i++)
			hash = (hash ^ (int)blockGrid.Blocks[i]) * 16777619;

		return hash;
#elif SIZE_MAX == UINT64_MAX
		size_t hash = 14695981039346656037;

		for (int i = 0; i < blockGrid.Width * blockGrid.Height; i++)
		    hash = (hash ^ (int)blockGrid.Blocks[i]) * 1099511628211;

		return hash;
#endif
	}
};

struct BlockGridEqualTo
{
	bool operator()(const BlockGrid& blockGrid1, const BlockGrid& blockGrid2) const
	{
		return blockGrid1.Equals(blockGrid2);
	}
};

class BlocSolver
{
	unsigned int smallestGroupSize;

	std::vector<std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>*> blockGrids;

	std::unique_ptr<BlockGrid> bestGrid;
	unsigned int bestScore;

	unsigned int depth;
	unsigned int worstNumberOfBlocks;
	unsigned int dbSize;

public:
	void Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, unsigned int maxDBSize, unsigned int maxDepth, bool save, bool dontAddToDBLastDepth);

private:
	void SolveDepth(unsigned int maxDBSize, bool& stop, bool dontAddToDB = false);
	unsigned int SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks,
		std::vector<std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>*>& newBlockGrids,
		unsigned int& newWorstNumberOfBlocks, bool& stop, bool dontAddToDB = false);
	bool IsInDatabase(const BlockGrid& blockGrid, unsigned int numberOfBlocks, std::vector<std::unordered_set<BlockGrid, BlockGridHash, BlockGridEqualTo>*>& newBlockGrids) const;
};