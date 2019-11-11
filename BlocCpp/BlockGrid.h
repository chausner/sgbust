#pragma once

#include <memory>
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

struct BlockGrid
{
	unsigned char Width;
	unsigned char Height;
	std::unique_ptr<BlockColor[]> Blocks;
	std::vector<unsigned char> Solution;

	BlockGrid(const std::string& path, unsigned int& smallestGroupSize);
	BlockGrid(const BlockGrid& blockGrid);
	BlockGrid(BlockGrid&& blockGrid) noexcept;
	BlockGrid& operator=(const BlockGrid& blockGrid);
	BlockGrid& operator=(BlockGrid&& blockGrid) noexcept;

	bool Equals(const BlockGrid& blockGrid) const;
	std::vector<std::vector<Position>> GetGroups(unsigned int smallestGroupSize) const;
	void RemoveGroup(const std::vector<Position>& group);
	std::string GetSolutionAsString() const;
	unsigned int GetNumberOfBlocks() const;

private:
	void GetAdjacentBlocksRecursive(std::vector<Position>& blockList, bool* flags, unsigned int x, unsigned int y) const;
	BlockColor* BlocksBegin() const { return Blocks.get(); } 
	BlockColor* BlocksEnd() const { return Blocks.get() + Width * Height; } 
};