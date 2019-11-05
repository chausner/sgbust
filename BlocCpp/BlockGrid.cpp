#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include "BlockGrid.h"

#define XY(x,y) ((y) * Width + (x))

BlockGrid::BlockGrid(const std::string& path, unsigned int& smallestGroupSize) : Solution()
{
	std::ifstream file(path, std::ifstream::binary);

	char header[4];

	file.read(&header[0], 4);

	if (std::string(&header[0], 4) != "BGF2")
		throw std::runtime_error("Invalid bloc grid file.");

	Width = file.get();
	Height = file.get();

	smallestGroupSize = file.get();

	Blocks = new BlockColor[Width * Height];

	file.read(reinterpret_cast<char*>(&Blocks[0]), Width * Height);
}

BlockGrid::BlockGrid(const BlockGrid& blockGrid)
{
	Width = blockGrid.Width;
	Height = blockGrid.Height;
	Solution = blockGrid.Solution;

	Blocks = new BlockColor[Width * Height];
	
	std::memcpy(&Blocks[0], &blockGrid.Blocks[0], Width * Height * sizeof(Blocks[0]));
}

BlockGrid::~BlockGrid()
{
	delete[] Blocks;
}

bool BlockGrid::Equals(const BlockGrid& blockGrid) const
{
	return Width == blockGrid.Width &&
		Height == blockGrid.Height &&
		memcmp(&Blocks[0], &blockGrid.Blocks[0], Width * Height * sizeof(Blocks[0])) == 0;
}

std::vector<std::vector<Position>> BlockGrid::GetGroups(unsigned int smallestGroupSize) const
{
	std::vector<std::vector<Position>> groups;

	bool* flags = static_cast<bool*>(alloca(Width * Height * sizeof(bool)));

	std::memset(flags, 0, Width * Height * sizeof(bool));

	std::vector<Position> adjacentBlocks;

	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
			if (!flags[XY(x, y)] && Blocks[XY(x, y)] != BlockColor::None)
			{
				if (x != Width - 1 && y != Height - 1 && Blocks[XY(x, y)] != Blocks[XY(x + 1, y)] && Blocks[XY(x, y)] != Blocks[XY(x, y + 1)])
				{
					flags[XY(x, y)] = true;
					continue;
				}

				GetAdjacentBlocksRecursive(adjacentBlocks, flags, Blocks[XY(x, y)], x, y);

				if (adjacentBlocks.size() >= smallestGroupSize)
					groups.push_back(adjacentBlocks);

				adjacentBlocks.clear();
			}

	return groups;
}

void BlockGrid::GetAdjacentBlocksRecursive(std::vector<Position>& blockList, bool* flags, BlockColor color, unsigned int x, unsigned int y) const
{
	flags[XY(x, y)] = true;

	blockList.emplace_back(x, y);

	if (x > 0 && !flags[XY(x - 1, y)] && Blocks[XY(x - 1, y)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, color, x - 1, y);
	if (y > 0 && !flags[XY(x, y - 1)] && Blocks[XY(x, y - 1)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, color, x, y - 1);
	if (x < Width - 1 && !flags[XY(x + 1, y)] && Blocks[XY(x + 1, y)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, color, x + 1, y);
	if (y < Height - 1 && !flags[XY(x, y + 1)] && Blocks[XY(x, y + 1)] == color)
		GetAdjacentBlocksRecursive(blockList, flags, color, x, y + 1);
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

	int newWidth = Width;

	for (int x = Width - 1; x >= 0; x--)
	{
		bool empty = true;

		for (int y = Height - 1; y >= 0; y--)
			if (Blocks[XY(x, y)] != BlockColor::None)
			{
				empty = false;
				break;
			}

		if (!empty)
		{
			newWidth = x + 1;
			break;
		}
	}

	int newHeight = Height;

	for (int y = 0; y < Height - 1; y++)
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
		BlockColor* blocks = new BlockColor[newWidth * newHeight];

		for (int y = 0; y < newHeight; y++)
			for (int x = 0; x < newWidth; x++)
				blocks[y * newWidth + x] = Blocks[XY(x, (Height - newHeight) + y)];

		Width = newWidth;
		Height = newHeight;

		delete[] Blocks;

		Blocks = blocks;
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
			solution.push_back('_');
			solution.push_back((char)((b / 26) + 64));
			solution.push_back((char)((b % 26) + 65));
			solution.push_back('_');
		}
	}

	return solution;
}

unsigned int BlockGrid::GetNumberOfBlocks() const
{
	unsigned int numberOfBlocks = 0;

	for (int i = 0; i < Width * Height; i++)
		if (Blocks[i] != BlockColor::None)
			numberOfBlocks++;

	return numberOfBlocks;
}

unsigned int BlockGrid::GetHash() const
{
	unsigned int hash = 0;

	for (int i = 0; i < Width * Height; i++)
	    hash += HASH_FACTORS[(unsigned char)Blocks[i]];

	return hash;
}