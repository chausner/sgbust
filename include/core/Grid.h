#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <memory>
#include <random>
#include <stdexcept>
#include <ostream>
#include <vector>

#include "core/Solution.h"
#include "experimental/mdspan"

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
	};

	struct Grid
	{
		using Extents = std::experimental::extents<unsigned int, std::experimental::dynamic_extent, std::experimental::dynamic_extent>;
		using BlocksSpan = std::experimental::mdspan<Block, Extents, std::experimental::layout_left>;
		using ConstBlocksSpan = std::experimental::mdspan<const Block, Extents, std::experimental::layout_left>;

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
		void GetGroups(std::vector<std::vector<Position>>& groups, unsigned int minGroupSize) const;
		void RemoveGroup(const std::vector<Position>& group);
		unsigned int GetNumberOfBlocks() const;
		void ApplySolution(const sgbust::Solution& solution, unsigned int minGroupSize);

		Block* BlocksBegin() { return Blocks.get(); }
		Block* BlocksEnd() { return Blocks.get() + Width * Height; }
		const Block* BlocksBegin() const { return Blocks.get(); }
		const Block* BlocksEnd() const { return Blocks.get() + Width * Height; }
		bool IsEmpty() const;
		BlocksSpan BlocksView() { return BlocksSpan(Blocks.get(), Width, Height); }
		ConstBlocksSpan BlocksView() const { return ConstBlocksSpan(Blocks.get(), Width, Height); }

		void Print() const;

	private:
		static void GetAdjacentBlocksRecursive(BlocksSpan blocks, std::vector<Position>& blockList, unsigned char x, unsigned char y);
	};

	template <typename Generator>
	Grid Grid::GenerateRandom(unsigned char width, unsigned char height, unsigned int numColors, Generator& generator)
	{
		if (numColors < 1 || numColors > 7)
			throw std::invalid_argument("numColors must be between 1 and 7");

		Grid grid(width, height);
		std::uniform_int_distribution<> dist(1, numColors);
		std::generate(grid.BlocksBegin(), grid.BlocksEnd(), [&] {
			return static_cast<Block>(dist(generator));
			});

		return grid;
	}
}