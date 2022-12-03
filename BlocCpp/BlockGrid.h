#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
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

	Position() {}
	Position(unsigned char x, unsigned char y) : X(x), Y(y) { }
};

struct Solution
{
	Solution() = default;
	Solution(std::string_view string);
	Solution(const Solution& solution);
	Solution(Solution&& solution) noexcept = default;
	Solution& operator=(const Solution& solution);
	Solution& operator=(Solution&& solution) noexcept = default;

	Solution Append(unsigned char step) const;
	Solution Append(const Solution& solution) const;
	std::string AsString() const;
	std::vector<unsigned char> AsVector() const { return std::vector(steps.get(), steps.get() + GetLength()); }
	unsigned int GetLength() const;
	bool IsEmpty() const { return steps == nullptr; }

	unsigned char operator[](unsigned int index) const { return steps[index]; }

private:
	std::unique_ptr<unsigned char[]> steps;
};

struct BlockGrid
{
	unsigned char Width;
	unsigned char Height;
	std::unique_ptr<BlockColor[]> Blocks;
	::Solution Solution;

	BlockGrid(unsigned char width, unsigned char height);
	BlockGrid(unsigned char width, unsigned char height, const BlockColor* blocks, ::Solution solution);
	BlockGrid(const std::string& path, unsigned int& smallestGroupSize);
	BlockGrid(const BlockGrid& blockGrid);
	BlockGrid(BlockGrid&& blockGrid) noexcept;
	BlockGrid& operator=(const BlockGrid& blockGrid);
	BlockGrid& operator=(BlockGrid&& blockGrid) noexcept;

	template <typename Generator>
	static BlockGrid GenerateRandom(unsigned char width, unsigned char height, unsigned int numColors, Generator& generator);

	void Save(std::ostream& stream, unsigned int smallestGroupSize) const;
	void GetGroups(std::vector<std::vector<Position>>& groups, unsigned int smallestGroupSize) const;
	void RemoveGroup(const std::vector<Position>& group);
	unsigned int GetNumberOfBlocks() const;
	void ApplySolution(const ::Solution& solution, unsigned int smallestGroupSize);

	BlockColor* BlocksBegin() { return Blocks.get(); }
	BlockColor* BlocksEnd() { return Blocks.get() + Width * Height; }
	const BlockColor* BlocksBegin() const { return Blocks.get(); }
	const BlockColor* BlocksEnd() const { return Blocks.get() + Width * Height; }
	bool IsEmpty() const;

	void Print() const;

private:
	unsigned int GetAdjacentBlocks(Position* blockList, unsigned char x, unsigned char y) const;
	void GetAdjacentBlocksRecursive(Position*& blockList, unsigned char x, unsigned char y) const;
};

template <typename Generator>
BlockGrid BlockGrid::GenerateRandom(unsigned char width, unsigned char height, unsigned int numColors, Generator& generator)
{
	if (numColors < 1 || numColors > 7)
		throw std::invalid_argument("numColors must be between 1 and 7");

	BlockGrid grid(width, height);
	std::uniform_int_distribution<> dist(1, numColors);
	std::generate(grid.BlocksBegin(), grid.BlocksEnd(), [&]() {
		return static_cast<BlockColor>(dist(generator));
	});
	
	return grid;
}