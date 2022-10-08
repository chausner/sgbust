#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <ostream>
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

struct Solution
{
	Solution() = default;
	Solution(const Solution& solution);
	Solution(Solution&& solution) noexcept = default;
	Solution& operator=(const Solution& solution);
	Solution& operator=(Solution&& solution) noexcept = default;

	Solution Append(unsigned char step);
	std::string AsString() const;
	std::vector<unsigned char> AsVector() const { return std::vector(steps.get(), steps.get() + GetLength()); }
	unsigned int GetLength() const;

private:
	std::unique_ptr<unsigned char[]> steps;
};

#pragma pack(push, 1)
struct BlockGrid
{
	unsigned char Width;
	unsigned char Height;
	std::unique_ptr<BlockColor[]> Blocks;
	::Solution Solution;

	BlockGrid(unsigned char width, unsigned char height);
	BlockGrid(const std::string& path, unsigned int& smallestGroupSize);
	BlockGrid(const BlockGrid& blockGrid);
	BlockGrid(BlockGrid&& blockGrid) noexcept;
	BlockGrid& operator=(const BlockGrid& blockGrid);
	BlockGrid& operator=(BlockGrid&& blockGrid) noexcept;

	void Save(std::ostream& stream, unsigned int smallestGroupSize) const;
	std::vector<std::vector<Position>> GetGroups(unsigned int smallestGroupSize) const;
	void RemoveGroup(const std::vector<Position>& group);
	unsigned int GetNumberOfBlocks() const;

	BlockColor* BlocksBegin() { return Blocks.get(); }
	BlockColor* BlocksEnd() { return Blocks.get() + Width * Height; }
	const BlockColor* BlocksBegin() const { return Blocks.get(); }
	const BlockColor* BlocksEnd() const { return Blocks.get() + Width * Height; }
	bool IsEmpty() const;

	void Print() const;

private:
	void GetAdjacentBlocksRecursive(std::vector<Position>& blockList, bool* flags, unsigned int x, unsigned int y) const;
};
#pragma pack(pop)