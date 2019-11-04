#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "BlockGrid.h"

class BlocSolver
{
	unsigned int smallestGroupSize;

	std::vector<std::vector<std::vector<BlockGrid>*>*> blockGrids;

	std::unique_ptr<BlockGrid> bestGrid;
	unsigned int bestScore;

	unsigned int depth;
	unsigned int worstNumberOfBlocks;
	unsigned int worstHash;
	unsigned int dbSize;

public:
	void Solve(BlockGrid& blockGrid, unsigned int smallestGroupSize, std::optional<unsigned int> maxDBSize, std::optional<unsigned int> maxDepth, bool save, bool dontAddToDBLastDepth);

private:
	void SolveDepth(std::optional<unsigned int> maxDBSize, bool& stop, bool dontAddToDB = false);
	unsigned int SolveBlockGrid(const BlockGrid& blockGrid, unsigned int numberOfBlocks, unsigned int hash,
		std::vector<std::vector<std::vector<BlockGrid>*>*>& newBlockGrids,
		unsigned int& newWorstNumberOfBlocks, unsigned int& newWorstHash, bool& stop, bool dontAddToDB = false);
	bool IsInDatabase(const BlockGrid& blockGrid, unsigned int numberOfBlocks, unsigned int hash, std::vector<std::vector<std::vector<BlockGrid>*>*>& newBlockGrids) const;
};