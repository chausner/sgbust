#pragma once

#include "BlockGrid.h"

#pragma pack(push, 1)
struct CompactBlockGrid
{
	unsigned char Width;
	unsigned char Height;
	std::unique_ptr<std::byte[]> Data;
	::Solution Solution;

	CompactBlockGrid(const CompactBlockGrid& blockGrid);
	CompactBlockGrid(CompactBlockGrid&& blockGrid) noexcept;
	CompactBlockGrid(const BlockGrid& blockGrid);
	CompactBlockGrid(BlockGrid&& blockGrid);
	CompactBlockGrid& operator=(const CompactBlockGrid& blockGrid);
	CompactBlockGrid& operator=(CompactBlockGrid&& blockGrid) noexcept;

	std::size_t DataLength() const;
	BlockGrid Expand() const;	

private:
	void Compact(const BlockGrid& blockGrid);
};
#pragma pack(pop)