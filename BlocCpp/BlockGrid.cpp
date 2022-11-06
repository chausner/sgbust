#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>
#include "BlockGrid.h"

#define XY(x,y) ((y) * Width + (x))

Solution::Solution(std::string_view string)
{
	if (string.empty())
		return;

	std::vector<unsigned char> steps;
	steps.reserve(string.length());

	for (auto it = string.begin(); it != string.end(); )
	{
		auto isAToZ = [](char c) { return c >= 'A' && c <= 'Z'; };

		if (isAToZ(it[0]))
		{
			steps.push_back(it[0] - 65);
			it++;
		}
		else if (it[0] == '(' && std::distance(it, string.end()) >= 4 && isAToZ(it[1]) && isAToZ(it[2]) && it[3] == ')')
		{
			unsigned int n = (it[1] - 64) * 26 + (it[2] - 65);
			if (n > 255)
				throw std::invalid_argument("Invalid solution string");
			steps.push_back(n);
			it += 4;
		}
		else
			throw std::invalid_argument("Invalid solution string");
	}

	this->steps = std::make_unique_for_overwrite<unsigned char[]>(steps.size() + 1);
	std::copy(steps.begin(), steps.end(), this->steps.get());
	this->steps.get()[steps.size()] = 0xFF;
}

Solution::Solution(const Solution& solution)
{
	if (solution.steps != nullptr)
	{
		int length = solution.GetLength();
		steps = std::make_unique_for_overwrite<unsigned char[]>(length + 1);
		std::copy(solution.steps.get(), solution.steps.get() + length + 1, steps.get());
	}
}

Solution& Solution::operator=(const Solution& solution)
{
	if (solution.steps != nullptr)
	{
		int length = solution.GetLength();
		steps = std::make_unique_for_overwrite<unsigned char[]>(length + 1);
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
		result.steps = std::make_unique_for_overwrite<unsigned char[]>(length + 2);
		std::copy(steps.get(), steps.get() + length, result.steps.get());
		result.steps[length] = step;
		result.steps[length + 1] = 0xFF;
	}
	else
	{		
		result.steps = std::make_unique_for_overwrite<unsigned char[]>(2);
		result.steps[0] = step;
		result.steps[1] = 0xFF;		
	}

	return result;
}

Solution Solution::Append(const Solution& solution) const
{
	if (solution.IsEmpty())
		return *this;
	else if (IsEmpty())
		return solution;
	else
	{
		Solution result;
		unsigned int length1 = GetLength();
		unsigned int length2 = solution.GetLength();
		result.steps = std::make_unique_for_overwrite<unsigned char[]>(length1 + length2 + 1);
		std::copy(steps.get(), steps.get() + length1, result.steps.get());
		std::copy(solution.steps.get(), solution.steps.get() + length2, result.steps.get() + length1);
		result.steps[length1 + length2] = 0xFF;
		return result;
	}
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

BlockGrid::BlockGrid(unsigned char width, unsigned char height, const BlockColor* blocks, ::Solution solution) : Width(width), Height(height), Blocks(std::make_unique_for_overwrite<BlockColor[]>(width * height)), Solution(std::move(solution))
{
	std::copy(blocks, blocks + Width * Height, BlocksBegin());
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

	Blocks = std::make_unique_for_overwrite<BlockColor[]>(Width * Height);

	file.read(reinterpret_cast<char*>(&Blocks[0]), Width * Height);
}

BlockGrid::BlockGrid(const BlockGrid& blockGrid) 
	: Width(blockGrid.Width), Height(blockGrid.Height), Solution(blockGrid.Solution)
{
	Blocks = std::make_unique_for_overwrite<BlockColor[]>(Width * Height);
	
	std::copy(blockGrid.BlocksBegin(), blockGrid.BlocksEnd(), BlocksBegin());
}

BlockGrid::BlockGrid(BlockGrid&& blockGrid) noexcept
	: Width(blockGrid.Width), Height(blockGrid.Height), Blocks(std::move(blockGrid.Blocks)), Solution(std::move(blockGrid.Solution))
{
}

BlockGrid& BlockGrid::operator=(const BlockGrid& blockGrid)
{
	if (Width * Height < blockGrid.Width * blockGrid.Height || Blocks == nullptr)
		Blocks = std::make_unique_for_overwrite<BlockColor[]>(blockGrid.Width * blockGrid.Height);

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

void BlockGrid::GetGroups(std::vector<std::vector<Position>>& groups, unsigned int smallestGroupSize) const
{
	groups.clear();
	groups.reserve(24);

	static thread_local std::vector<Position> adjacentBlocks;
	adjacentBlocks.resize(Width * Height);

	for (unsigned char y = 0; y < Height; y++)
		for (unsigned char x = 0; x < Width; x++)
			if (static_cast<unsigned char>(Blocks[XY(x, y)]) < 128 && Blocks[XY(x, y)] != BlockColor::None)
			{
				if (smallestGroupSize > 1)
					if (x != Width - 1 && y != Height - 1 && Blocks[XY(x, y)] != Blocks[XY(x + 1, y)] && Blocks[XY(x, y)] != Blocks[XY(x, y + 1)])
						continue;

				unsigned int numAdjacentBlocks = GetAdjacentBlocks(adjacentBlocks.data(), x, y);

				if (numAdjacentBlocks >= smallestGroupSize)
					groups.emplace_back(adjacentBlocks.begin(), adjacentBlocks.begin() + numAdjacentBlocks);
			}

	for (int i = 0; i < Width * Height; i++)
		if (static_cast<unsigned char>(Blocks[i]) >= 128)
			Blocks[i] = static_cast<BlockColor>(~static_cast<unsigned char>(Blocks[i]));
}

unsigned int BlockGrid::GetAdjacentBlocks(Position* blockList, unsigned char x, unsigned char y) const
{
	Position* p = blockList;
	GetAdjacentBlocksRecursive(p, x, y);

	size_t numAdjacentBlocks = p - blockList;
	return numAdjacentBlocks;
}

void BlockGrid::GetAdjacentBlocksRecursive(Position*& blockList, unsigned char x, unsigned char y) const
{
	*blockList = Position(x, y);
	blockList++;

	BlockColor color = Blocks[XY(x, y)];

	Blocks[XY(x, y)] = static_cast<BlockColor>(~static_cast<unsigned char>(Blocks[XY(x, y)]));

	if (x > 0 && Blocks[XY(x - 1, y)] == color)
		GetAdjacentBlocksRecursive(blockList, x - 1, y);
	if (y > 0 && Blocks[XY(x, y - 1)] == color)
		GetAdjacentBlocksRecursive(blockList, x, y - 1);
	if (x < Width - 1 && Blocks[XY(x + 1, y)] == color)
		GetAdjacentBlocksRecursive(blockList, x + 1, y);
	if (y < Height - 1 && Blocks[XY(x, y + 1)] == color)
		GetAdjacentBlocksRecursive(blockList, x, y + 1);
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
		std::unique_ptr<BlockColor[]> newBlocks = std::make_unique_for_overwrite<BlockColor[]>(newWidth * newHeight);

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

void BlockGrid::ApplySolution(const ::Solution& solution, unsigned int smallestGroupSize)
{
	unsigned int length = solution.GetLength();

	std::vector<std::vector<Position>> groups;

	for (unsigned int i = 0; i < length; i++)
	{
		GetGroups(groups, smallestGroupSize);
		if (solution[i] >= groups.size())
			throw std::runtime_error("Invalid solution");
		RemoveGroup(groups[solution[i]]);
	}
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