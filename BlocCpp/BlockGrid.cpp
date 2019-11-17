#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include "BlockGrid.h"

#define XY(x,y) ((y) * Width + (x))

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
	: Width(blockGrid.Width), Height(blockGrid.Height)
{
	// optimize for BlocSolver which calls this copy-constructor, followed by a Solution.push_back
	Solution.reserve(blockGrid.Solution.size() + 1);
	Solution.resize(blockGrid.Solution.size());
	std::copy(blockGrid.Solution.begin(), blockGrid.Solution.end(), Solution.begin());

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

	for (Position p : group)
	{
		if (p.X < left)
			left = p.X;
		if (p.X > right)
			right = p.X;
		if (p.Y > bottom)
			bottom = p.Y;

		int x = p.X;
		int y = p.Y;

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
	}

	int newWidth;

	if (bottom != Height - 1)
		newWidth = Width;
	else
	{
		newWidth = 0;
		for (int x = Width - 1; x >= 0; x--)
			if (Blocks[XY(x, Height - 1)] != BlockColor::None)
			{
				newWidth = x + 1;
				break;
			}
	}

	int newHeight = 0;

	for (int y = 0; y < Height; y++)
	{
		bool empty = true;

		for (int x = 0; x < newWidth; x++)
			if (Blocks[XY(x, y)] != BlockColor::None)
			{
				empty = false;
				break;
			}

		if (!empty)
		{
			newHeight = Height - y;
			break;
		}
	}

	if (newWidth != Width || newHeight != Height)
	{
		std::unique_ptr<BlockColor[]> blocks = std::make_unique<BlockColor[]>(newWidth * newHeight);

		for (int y = 0; y < newHeight; y++)
			for (int x = 0; x < newWidth; x++)
				blocks[y * newWidth + x] = Blocks[XY(x, (Height - newHeight) + y)];

		Width = newWidth;
		Height = newHeight;
		Blocks = std::move(blocks);
	}
}

std::string BlockGrid::GetSolutionAsString() const
{
	std::string solution;

	for (unsigned char b : Solution)
	{
		if (b < 26)
			solution.push_back((char)(b + 65));
		else
		{
			solution.push_back('(');
			solution.push_back((char)((b / 26) + 64));
			solution.push_back((char)((b % 26) + 65));
			solution.push_back(')');
		}
	}

	return solution;
}

unsigned int BlockGrid::GetNumberOfBlocks() const
{
	return std::count_if(BlocksBegin(), BlocksEnd(), [](auto c) { return c != BlockColor::None; });
}
