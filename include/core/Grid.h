#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <mdspan>
#include <memory>
#include <random>
#include <ranges>
#include <stdexcept>
#include <ostream>
#include <vector>

#include "core/Solution.h"

namespace sgbust
{
    enum class Block : unsigned char
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

        bool operator==(const Position& other) const = default;
    };

    using Group = std::vector<Position>;

    struct Grid
    {
        using Extents = std::extents<unsigned int, std::dynamic_extent, std::dynamic_extent>;
        using BlocksSpan = std::mdspan<Block, Extents, std::layout_left>;
        using ConstBlocksSpan = std::mdspan<const Block, Extents, std::layout_left>;

        unsigned char Width;
        unsigned char Height;
        std::unique_ptr<Block[]> Blocks;
        sgbust::Solution Solution;

        Grid(unsigned char width, unsigned char height);
        Grid(unsigned char width, unsigned char height, const Block* blocks, sgbust::Solution solution);
        Grid(std::istream& stream, unsigned int& minGroupSize);
        Grid(const Grid& grid);
        Grid(Grid&& grid) noexcept;
        Grid& operator=(const Grid& grid);
        Grid& operator=(Grid&& grid) noexcept;

        template <typename Generator>
        static Grid GenerateRandom(unsigned char width, unsigned char height, unsigned int numColors, Generator& generator);

        void Save(std::ostream& stream, unsigned int minGroupSize) const;
        void GetGroups(std::vector<Group>& groups, unsigned int minGroupSize) const;
        bool HasGroups(unsigned int minGroupSize) const;
        void RemoveGroup(const Group& group);
        unsigned int GetNumberOfBlocks() const;
        std::array<unsigned int, 8> GetColorCounts() const;
        unsigned int GetNumberOfColors() const;
        void ApplySolution(const sgbust::Solution& solution, unsigned int minGroupSize);

        Block* BlocksBegin() { return Blocks.get(); }
        Block* BlocksEnd() { return Blocks.get() + Width * Height; }
        const Block* BlocksBegin() const { return Blocks.get(); }
        const Block* BlocksEnd() const { return Blocks.get() + Width * Height; }
        bool IsEmpty() const;
        BlocksSpan BlocksView() { return BlocksSpan(Blocks.get(), Width, Height); }
        ConstBlocksSpan BlocksView() const { return ConstBlocksSpan(Blocks.get(), Width, Height); }
        std::ranges::subrange<Block*> BlocksRange() { return std::ranges::subrange<Block*>(BlocksBegin(), BlocksEnd()); }
        std::ranges::subrange<const Block*> BlocksRange() const { return std::ranges::subrange<const Block*>(BlocksBegin(), BlocksEnd()); }

        void Print(const Group* highlightedGroup = nullptr) const;

    private:
        static void GetAdjacentBlocksRecursive(BlocksSpan blocks, Group& group, unsigned char x, unsigned char y);
        void ShrinkToFit(unsigned char newWidth, unsigned char newHeight);
    };

    template <typename Generator>
    Grid Grid::GenerateRandom(unsigned char width, unsigned char height, unsigned int numColors, Generator& generator)
    {
        if (numColors < 1 || numColors > 7)
            throw std::invalid_argument("numColors must be between 1 and 7");

        Grid grid(width, height);
        std::uniform_int_distribution<> dist(1, numColors);
        std::ranges::generate(grid.BlocksRange(), [&] {
            return static_cast<Block>(dist(generator));
            });

        return grid;
    }
}