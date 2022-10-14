#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include "BlockGrid.h"

#define XY(x,y) ((y) * Width + (x))

Solution::Solution(const Solution& solution)
{
	if (solution.steps != nullptr)
	{
		int length = solution.GetLength();
		steps = std::make_unique<unsigned char[]>(length + 1);
		std::copy(solution.steps.get(), solution.steps.get() + length + 1, steps.get());
	}
}

Solution& Solution::operator=(const Solution& solution)
{
	if (solution.steps != nullptr)
	{
		int length = solution.GetLength();
		steps = std::make_unique<unsigned char[]>(length + 1);
		std::copy(solution.steps.get(), solution.steps.get() + length + 1, steps.get());
	}
	else
		steps = nullptr;

	return *this;
}

Solution Solution::Append(unsigned char step) const
{
	Solution result;

	if (steps != nullptr)
	{
		int length = GetLength();
		result.steps = std::make_unique<unsigned char[]>(length + 2);
		std::copy(steps.get(), steps.get() + length, result.steps.get());
		result.steps[length] = step;
		result.steps[length + 1] = 0xFF;
	}
	else
	{		
		result.steps = std::make_unique<unsigned char[]>(2);
		result.steps[0] = step;
		result.steps[1] = 0xFF;		
	}

	return result;
}

std::string Solution::AsString() const
{
	if (steps == nullptr)
		return std::string();
	
	std::string solution;

	for (unsigned char* step = steps.get(); *step != 0xFF; step++)
	{
		if (*step < 26)
			solution.push_back((char)(*step + 65));
		else
		{
			solution.push_back('(');
			solution.push_back((char)((*step / 26) + 64));
			solution.push_back((char)((*step % 26) + 65));
			solution.push_back(')');
		}
	}

	return solution;
}

unsigned int Solution::GetLength() const
{
	if (steps == nullptr)
		return 0;

	int length = 0;

	while (steps[length] != 0xFF)
		length++;

	return length;
}

BlockGrid::BlockGrid(unsigned char width, unsigned char height) : Width(width), Height(height), Blocks(std::make_unique<BlockColor[]>(width * height))
{
}

BlockGrid::BlockGrid(const std::string& path, unsigned int& smallestGroupSize)
{
	std::ifstream file(path, std::ifstream::binary);

	char header[4];

	file.read(&header[0], 4);

	if (std::string(&header[0], 4) != "BGF2")
		throw std::runtime_error("Invalid bloc grid file.");

	Width = file.get();
	Height = file.get();

	smallestGroupSize = file.get();

	Blocks = std::make_unique<BlockColor[]>(Width * Height);

	file.read(reinterpret_cast<char*>(&Blocks[0]), Width * Height);
}

BlockGrid::BlockGrid(const BlockGrid& blockGrid) 
	: Width(blockGrid.Width), Height(blockGrid.Height), Solution(blockGrid.Solution)
{
	Blocks = std::make_unique<BlockColor[]>(Width * Height);
	
	std::copy(blockGrid.BlocksBegin(), blockGrid.BlocksEnd(), BlocksBegin());
}

BlockGrid::BlockGrid(BlockGrid&& blockGrid) noexcept
	: Width(blockGrid.Width), Height(blockGrid.Height), Blocks(std::move(blockGrid.Blocks)), Solution(std::move(blockGrid.Solution))
{
}

BlockGrid& BlockGrid::operator=(const BlockGrid& blockGrid)
{
	if (Width * Height < blockGrid.Width * blockGrid.Height || Blocks == nullptr)
		Blocks = std::make_unique<BlockColor[]>(blockGrid.Width * blockGrid.Height);

	Width = blockGrid.Width;
	Height = blockGrid.Height;
	Solution = blockGrid.Solution;

	std::copy(blockGrid.BlocksBegin(), blockGrid.BlocksEnd(), BlocksBegin());

	return *this;
}

BlockGrid& BlockGrid::operator=(BlockGrid&& blockGrid) noexcept
{
	Width = blockGrid.Width;
	Height = blockGrid.Height;
	Blocks = std::move(blockGrid.Blocks);
	Solution = std::move(blockGrid.Solution);	

	return *this;
}

void BlockGrid::Save(std::ostream& stream, unsigned int smallestGroupSize) const
{
	stream << "BGF2";
	stream << Width;
	stream << Height;
	stream << static_cast<unsigned char>(smallestGroupSize);
	for (const BlockColor* b = BlocksBegin(); b != BlocksEnd(); b++)
		stream << static_cast<unsigned char>(*b);
}

std::vector<std::vector<Position>> BlockGrid::GetGroups(unsigned int smallestGroupSize) const
{
	std::vector<std::vector<Position>> groups;
	groups.reserve(24);

	std::unique_ptr<bool[]> flags = std::make_unique<bool[]>(Width * Height);

	std::vector<Position> adjacentBlocks;
	adjacentBlocks.reserve(24);

	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
			if (!flags[XY(x, y)] && Blocks[XY(x, y)] != BlockColor::None)
			{
				if (smallestGroupSize > 1)
					if (x != Width - 1 && y != Height - 1 && Blocks[XY(x, y)] != Blocks[XY(x + 1, y)] && Blocks[XY(x, y)] != Blocks[XY(x, y + 1)])
					{
						flags[XY(x, y)] = true;
						continue;
					}

				GetAdjacentBlocksRecursive(adjacentBlocks, flags.get(), x, y);

				if (adjacentBlocks.size() >= smallestGroupSize)
					groups.push_back(adjacentBlocks);

				adjacentBlocks.clear();
			}

	return groups;
}

void BlockGrid::GetAdjacentBlocksRecursive(std::vector<Position>& blockList, bool* flags, unsigned int x, unsigned int y) const
{
	flags[XY(x, y)] = true;

	blockList.emplace_back(x, y);

	BlockColor color = Blocks[XY(x, y)];

	if (x > 0 && !flags[XY(x - 1, y)] && Blocks[XY(x - 1, y)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, x - 1, y);
	if (y > 0 && !flags[XY(x, y - 1)] && Blocks[XY(x, y - 1)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, x, y - 1);
	if (x < Width - 1 && !flags[XY(x + 1, y)] && Blocks[XY(x + 1, y)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, x + 1, y);
	if (y < Height - 1 && !flags[XY(x, y + 1)] && Blocks[XY(x, y + 1)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, x, y + 1);
}

void BlockGrid::RemoveGroup(const std::vector<Position>& group)
{
	int left = std::numeric_limits<int>::max();
	int right = std::numeric_limits<int>::min();
	int bottom = std::numeric_limits<int>::min();

	for (auto [x, y] : group)
	{
		if (x < left)
			left = x;
		if (x > right)
			right = x;
		if (y > bottom)
			bottom = y;

		Blocks[XY(x, y)] = BlockColor::None;
	}
	
	for (int x = left; x <= right; x++)
	{
		int yy = bottom;

		for (int y = bottom; y >= 0; y--)
			if (Blocks[XY(x, y)] != BlockColor::None)
			{
				if (yy != y)
				{
					Blocks[XY(x, yy)] = Blocks[XY(x, y)];
					Blocks[XY(x, y)] = BlockColor::None;
				}

				yy--;
			}
	}

	int newWidth = Width;

	if (bottom == Height - 1)
	{
		int xx = left;

		for (int x = left; x < Width; x++)
			if (Blocks[XY(x, Height - 1)] != BlockColor::None)
			{
				if (xx != x)
					for (int y = 0; y < Height; y++)
					{
						Blocks[XY(xx, y)] = Blocks[XY(x, y)];
						Blocks[XY(x, y)] = BlockColor::None;
					}

				xx++;
			}

		newWidth = xx;
	}

	BlockColor* firstBlock = std::find_if(BlocksBegin(), BlocksEnd(), [](auto b) { return b != BlockColor::None; });
	int newHeight = Height - std::distance(BlocksBegin(), firstBlock) / Width;

	if (newWidth != Width || newHeight != Height)
	{
		std::unique_ptr<BlockColor[]> newBlocks = std::make_unique<BlockColor[]>(newWidth * newHeight);

		if (newWidth != Width)
			for (int y = 0; y < newHeight; y++)
				for (int x = 0; x < newWidth; x++)
					newBlocks[y * newWidth + x] = Blocks[XY(x, (Height - newHeight) + y)];
		else
			std::copy(&Blocks[XY(0, Height - newHeight)], BlocksEnd(), newBlocks.get());

		Width = newWidth;
		Height = newHeight;
		Blocks = std::move(newBlocks);
	}
}

unsigned int BlockGrid::GetNumberOfBlocks() const
{
	return std::count_if(BlocksBegin(), BlocksEnd(), [](auto c) { return c != BlockColor::None; });
}

bool BlockGrid::IsEmpty() const
{
	return std::all_of(BlocksBegin(), BlocksEnd(), [](auto c) { return c == BlockColor::None; });
}

void BlockGrid::Print() const
{
	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			const char* colorCode;

			switch (Blocks[y * Width + x])
			{
			case BlockColor::None:
				colorCode = "\x1B[40m";
				break;
			case BlockColor::Black:
				colorCode = "\x1B[47m";
				break;
			case BlockColor::Red:
				colorCode = "\x1B[41m";
				break;
			case BlockColor::Green:
				colorCode = "\x1B[42m";
				break;
			case BlockColor::Blue:
				colorCode = "\x1B[44m";
				break;
			case BlockColor::Magenta:
				colorCode = "\x1B[45m";
				break;
			case BlockColor::Yellow:
				colorCode = "\x1B[43m";
				break;
			case BlockColor::Cyan:
				colorCode = "\x1B[46m";
				break;
			}

			std::cout << colorCode << "  ";
		}			

		std::cout << "\x1B[0m" << std::endl;
	}
}