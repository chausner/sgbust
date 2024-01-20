#include "core/Grid.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

namespace
{
    static constexpr char BlockVisited = 0b10000000;
}

namespace sgbust
{
    Grid::Grid(unsigned char width, unsigned char height) : Width(width), Height(height), Blocks(std::make_unique<Block[]>(width* height))
    {
    }

    Grid::Grid(unsigned char width, unsigned char height, const Block* blocks, sgbust::Solution solution)
        : Width(width), Height(height), Blocks(std::make_unique_for_overwrite<Block[]>(width* height)), Solution(std::move(solution))
    {
        std::copy(blocks, blocks + Width * Height, BlocksBegin());
    }

    Grid::Grid(std::istream& stream, unsigned int& minGroupSize)
    {
        std::string header;
        header.resize(4);

        stream.read(header.data(), header.size());

        if (stream.fail())
            throw std::runtime_error("Could not read stream.");

        if (header != "BGF2")
            throw std::runtime_error("Invalid Bloc Grid File: header corrupted");

        Width = stream.get();
        Height = stream.get();

        if ((Width == 0) != (Height == 0))
            throw std::runtime_error("Invalid Bloc Grid File: width/height invalid.");

        minGroupSize = stream.get();

        if (minGroupSize < 1)
            throw std::runtime_error("Invalid Bloc Grid File: minimal group size out-of-range.");

        Blocks = std::make_unique_for_overwrite<Block[]>(Width * Height);

        stream.read(reinterpret_cast<char*>(Blocks.get()), Width * Height);

        if (stream.fail())
            throw std::runtime_error("Could not read stream.");
    }

    Grid::Grid(const Grid& grid)
        : Width(grid.Width), Height(grid.Height), Solution(grid.Solution)
    {
        Blocks = std::make_unique_for_overwrite<Block[]>(Width * Height);

        std::copy(grid.BlocksBegin(), grid.BlocksEnd(), BlocksBegin());
    }

    Grid::Grid(Grid&& grid) noexcept
        : Width(grid.Width), Height(grid.Height), Blocks(std::move(grid.Blocks)), Solution(std::move(grid.Solution))
    {
    }

    Grid& Grid::operator=(const Grid& grid)
    {
        if (Width * Height < grid.Width * grid.Height || Blocks == nullptr)
            Blocks = std::make_unique_for_overwrite<Block[]>(grid.Width * grid.Height);

        Width = grid.Width;
        Height = grid.Height;
        Solution = grid.Solution;

        std::copy(grid.BlocksBegin(), grid.BlocksEnd(), BlocksBegin());

        return *this;
    }

    Grid& Grid::operator=(Grid&& grid) noexcept
    {
        Width = grid.Width;
        Height = grid.Height;
        Blocks = std::move(grid.Blocks);
        Solution = std::move(grid.Solution);

        return *this;
    }

    void Grid::Save(std::ostream& stream, unsigned int minGroupSize) const
    {
        stream << "BGF2";
        stream << Width;
        stream << Height;
        stream << static_cast<unsigned char>(minGroupSize);
        for (const Block* b = BlocksBegin(); b != BlocksEnd(); b++)
            stream << static_cast<unsigned char>(*b);

        if (stream.fail())
            throw std::runtime_error("Could not save Grid to stream");
    }

    void Grid::GetGroups(std::vector<Group>& groups, unsigned int minGroupSize) const
    {
        auto blocks = const_cast<Grid*>(this)->BlocksView();

        groups.clear();
        groups.reserve(24);

        static thread_local std::vector<Position> adjacentBlocks;
        adjacentBlocks.reserve(Width * Height);

        for (unsigned char y = 0; y < Height; y++)
            for (unsigned char x = 0; x < Width; x++)
                if ((static_cast<char>(blocks(x, y)) & BlockVisited) == 0 && blocks(x, y) != Block::None)
                {
                    if (minGroupSize > 1)
                        if (x != Width - 1 && y != Height - 1 && blocks(x, y) != blocks(x + 1, y) && blocks(x, y) != blocks(x, y + 1))
                            continue;

                    adjacentBlocks.clear();
                    GetAdjacentBlocksRecursive(blocks, adjacentBlocks, x, y);

                    if (adjacentBlocks.size() >= minGroupSize)
                        groups.emplace_back(adjacentBlocks);                    
                }

        for (auto it = const_cast<Grid*>(this)->BlocksBegin(), end = const_cast<Grid*>(this)->BlocksEnd(); it != end; it++)
            reinterpret_cast<char&>(*it) &= ~BlockVisited;
    }

    bool Grid::HasGroups(unsigned int minGroupSize) const
    {
        if (minGroupSize <= 1)
            return !IsEmpty();

        auto blocks = const_cast<Grid*>(this)->BlocksView();

        static thread_local std::vector<Position> adjacentBlocks;
        adjacentBlocks.reserve(Width * Height);

        bool hasGroups = false;

        for (unsigned char y = 0; y < Height; y++)
            for (unsigned char x = 0; x < Width; x++)
                if ((static_cast<char>(blocks(x, y)) & BlockVisited) == 0 && blocks(x, y) != Block::None)
                {
                    if (x != Width - 1 && y != Height - 1 && blocks(x, y) != blocks(x + 1, y) && blocks(x, y) != blocks(x, y + 1))
                        continue;

                    adjacentBlocks.clear();
                    GetAdjacentBlocksRecursive(blocks, adjacentBlocks, x, y);

                    if (adjacentBlocks.size() >= minGroupSize)
                    {
                        hasGroups = true;
                        goto exit;
                    }
                }

    exit:
        for (auto it = const_cast<Grid*>(this)->BlocksBegin(), end = const_cast<Grid*>(this)->BlocksEnd(); it != end; it++)
            reinterpret_cast<char&>(*it) &= ~BlockVisited;

        return hasGroups;
    }

    void Grid::GetAdjacentBlocksRecursive(BlocksSpan blocks, std::vector<Position>& blockList, unsigned char x, unsigned char y)
    {
        blockList.emplace_back(x, y);

        Block color = blocks(x, y);

        reinterpret_cast<char&>(blocks(x, y)) |= BlockVisited;

        if (x > 0 && blocks(x - 1, y) == color)
            GetAdjacentBlocksRecursive(blocks, blockList, x - 1, y);
        if (y > 0 && blocks(x, y - 1) == color)
            GetAdjacentBlocksRecursive(blocks, blockList, x, y - 1);
        if (x < blocks.extent(0) - 1 && blocks(x + 1, y) == color)
            GetAdjacentBlocksRecursive(blocks, blockList, x + 1, y);
        if (y < blocks.extent(1) - 1 && blocks(x, y + 1) == color)
            GetAdjacentBlocksRecursive(blocks, blockList, x, y + 1);
    }

    void Grid::RemoveGroup(const Group& group)
    {
        auto blocks = BlocksView();

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

            blocks(x, y) = Block::None;
        }

        for (int x = left; x <= right; x++)
        {
            int yy = bottom;

            for (int y = bottom; y >= 0; y--)
                if (blocks(x, y) != Block::None)
                {
                    if (yy != y)
                    {
                        blocks(x, yy) = blocks(x, y);
                        blocks(x, y) = Block::None;
                    }

                    yy--;
                }
        }

        int newWidth = Width;

        if (bottom == Height - 1)
        {
            int xx = left;

            for (int x = left; x < Width; x++)
                if (blocks(x, Height - 1) != Block::None)
                {
                    if (xx != x)
                        for (int y = 0; y < Height; y++)
                        {
                            blocks(xx, y) = blocks(x, y);
                            blocks(x, y) = Block::None;
                        }

                    xx++;
                }

            newWidth = xx;
        }

        Block* firstBlock = std::find_if(BlocksBegin(), BlocksEnd(), [](auto b) { return b != Block::None; });
        int newHeight = Height - std::distance(BlocksBegin(), firstBlock) / Width;

        if (newWidth != Width || newHeight != Height)
        {
            std::unique_ptr<Block[]> newBlocks = std::make_unique_for_overwrite<Block[]>(newWidth * newHeight);
            BlocksSpan newBlocksView(newBlocks.get(), newWidth, newHeight);

            if (newWidth != Width)
                for (int y = 0; y < newHeight; y++)
                    for (int x = 0; x < newWidth; x++)
                        newBlocksView(x, y) = blocks(x, (Height - newHeight) + y);
            else
                std::copy(&blocks(0, Height - newHeight), BlocksEnd(), newBlocks.get());

            Width = newWidth;
            Height = newHeight;
            Blocks = std::move(newBlocks);
        }
    }

    unsigned int Grid::GetNumberOfBlocks() const
    {
        return std::count_if(BlocksBegin(), BlocksEnd(), [](auto c) { return c != Block::None; });
    }

    unsigned int Grid::GetNumberOfColors() const
    {
        std::array<unsigned int, 8> colorCounts{};
        std::for_each(BlocksBegin(), BlocksEnd(), [&](Block c) {
            colorCounts[static_cast<int>(c)]++;
            });
        return std::count_if(colorCounts.begin() + 1, colorCounts.end(), [](unsigned int count) { return count != 0; });
    }

    void Grid::ApplySolution(const sgbust::Solution& solution, unsigned int minGroupSize)
    {
        unsigned int length = solution.GetLength();

        std::vector<Group> groups;

        for (unsigned int i = 0; i < length; i++)
        {
            GetGroups(groups, minGroupSize);
            if (solution[i] >= groups.size())
                throw std::runtime_error("Invalid solution");
            RemoveGroup(groups[solution[i]]);
        }
    }

    bool Grid::IsEmpty() const
    {
        return std::all_of(BlocksBegin(), BlocksEnd(), [](auto c) { return c == Block::None; });
    }

    void Grid::Print() const
    {
        auto blocks = BlocksView();

        for (int y = 0; y < Height; y++)
        {
            for (int x = 0; x < Width; x++)
            {
                const char* colorCode;

                switch (blocks(x, y))
                {
                case Block::None:
                    colorCode = "\x1B[40m";
                    break;
                case Block::Black:
                    colorCode = "\x1B[47m";
                    break;
                case Block::Red:
                    colorCode = "\x1B[41m";
                    break;
                case Block::Green:
                    colorCode = "\x1B[42m";
                    break;
                case Block::Blue:
                    colorCode = "\x1B[44m";
                    break;
                case Block::Magenta:
                    colorCode = "\x1B[45m";
                    break;
                case Block::Yellow:
                    colorCode = "\x1B[43m";
                    break;
                case Block::Cyan:
                    colorCode = "\x1B[46m";
                    break;
                default:
                    throw std::out_of_range("Unexpected block color value");
                }

                std::cout << colorCode << "  ";
            }

            std::cout << "\x1B[0m" << std::endl;
        }
    }
}