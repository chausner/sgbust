#pragma once

#include <string>
#include <vector>

enum class BlockColor : unsigned char
{
	None,
	Black,
	Red,
	Green,
	Blue,
	Magenta,
	Yellow,
	Cyan
};

struct Position
{
	unsigned char X;
	unsigned char Y;

	Position(unsigned char x, unsigned char y) : X(x), Y(y) { }
};

// factor for one BlockColor can be zero because total number of blocks is already stored separately
const unsigned int h = 8;
const unsigned int HASH_FACTORS[8] = { 0, 0, 1, h, h * h, h * h * h, h * h * h * h, h * h * h * h * h };

struct BlockGrid
{
	unsigned char Width;
	unsigned char Height;
	BlockColor* Blocks;
	std::vector<unsigned char> Solution;

	BlockGrid(const std::string& path, unsigned int& smallestGroupSize);
	BlockGrid(const BlockGrid& blockGrid);
	~BlockGrid();

	bool Equals(const BlockGrid& blockGrid) const;
	std::vector<std::vector<Position>> GetGroups(unsigned int smallestGroupSize) const;
	void RemoveGroup(const std::vector<Position>& group);
	std::string GetSolutionAsString() const;
	unsigned int GetNumberOfBlocks() const;
	unsigned int GetHash() const;

private:
	void GetAdjacentBlocksRecursive(std::vector<Position>& blockList, bool* flags, BlockColor color, unsigned int x, unsigned int y) const;
};