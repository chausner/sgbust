#pragma once

#include <cstddef>
#include <memory>

#include "core/Grid.h"

namespace sgbust
{
#pragma pack(push, 1)
	struct CompactGrid
	{
		unsigned char Width;
		unsigned char Height;
		std::unique_ptr<std::byte[]> Data;
		sgbust::Solution Solution;

		CompactGrid();
		CompactGrid(const CompactGrid& grid);
		CompactGrid(CompactGrid&& grid) noexcept;
		CompactGrid(const Grid& grid);
		CompactGrid(Grid&& grid);
		CompactGrid& operator=(const CompactGrid& grid);
		CompactGrid& operator=(CompactGrid&& grid) noexcept;

		std::size_t DataLength() const;
		Grid Expand() const;

	private:
		void Compact(const Grid& grid);
	};
#pragma pack(pop)
}