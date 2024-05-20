#include "Grid.h"
#include "CompactGrid.h"

namespace bloc
{
	CompactGrid::CompactGrid() : Width(0), Height(0)
	{
	}

	CompactGrid::CompactGrid(const CompactGrid& grid) : Width(grid.Width), Height(grid.Height), Solution(grid.Solution)
	{
		Data = std::make_unique_for_overwrite<std::byte[]>(grid.DataLength());
		std::copy(grid.Data.get(), grid.Data.get() + grid.DataLength(), Data.get());
	}

	CompactGrid::CompactGrid(CompactGrid&& grid) noexcept : Width(grid.Width), Height(grid.Height), Data(std::move(grid.Data)), Solution(std::move(grid.Solution))
	{
	}

	CompactGrid::CompactGrid(const Grid& grid) : Width(grid.Width), Height(grid.Height), Solution(grid.Solution)
	{
		Compact(grid);
	}

	CompactGrid::CompactGrid(Grid&& grid) : Width(grid.Width), Height(grid.Height), Solution(std::move(grid.Solution))
	{
		Compact(grid);
	}

	CompactGrid& CompactGrid::operator=(const CompactGrid& grid)
	{
		Width = grid.Width;
		Height = grid.Height;
		Data = std::make_unique_for_overwrite<std::byte[]>(grid.DataLength());
		std::copy(grid.Data.get(), grid.Data.get() + grid.DataLength(), Data.get());
		Solution = grid.Solution;
		return *this;
	}

	CompactGrid& CompactGrid::operator=(CompactGrid&& grid) noexcept
	{
		Width = grid.Width;
		Height = grid.Height;
		Data = std::move(grid.Data);
		Solution = std::move(grid.Solution);
		return *this;
	}

	std::size_t CompactGrid::DataLength() const
	{
		return (Width * Height * 3 + 7) / 8;
	}

	Grid CompactGrid::Expand() const
	{
		Grid grid(Width, Height);
		grid.Solution = Solution;

		Block* blocks = grid.BlocksBegin();

		for (int i = 0; i + 7 < Width * Height; i += 8)
		{
			const std::byte* b = &Data[i / 8 * 3];
			blocks[i] = static_cast<Block>(b[0] & std::byte{ 0b111 });
			blocks[i + 1] = static_cast<Block>((b[0] >> 3) & std::byte{ 0b111 });
			blocks[i + 2] = static_cast<Block>(((b[0] >> 5) & std::byte{ 0b110 }) | (b[1] & std::byte{ 0b001 }));
			blocks[i + 3] = static_cast<Block>((b[1] >> 1) & std::byte{ 0b111 });
			blocks[i + 4] = static_cast<Block>((b[1] >> 4) & std::byte{ 0b111 });
			blocks[i + 5] = static_cast<Block>(((b[1] >> 5) & std::byte{ 0b100 }) | (b[2] & std::byte{ 0b011 }));
			blocks[i + 6] = static_cast<Block>((b[2] >> 2) & std::byte{ 0b111 });
			blocks[i + 7] = static_cast<Block>((b[2] >> 5) & std::byte{ 0b111 });
		}

		for (int i = Width * Height - (Width * Height % 8); i < Width * Height; i++)
		{
			const std::byte* b = &Data[i / 8 * 3];
			switch (i % 8)
			{
			case 0:
				blocks[i] = static_cast<Block>(b[0] & std::byte{ 0b111 });
				break;
			case 1:
				blocks[i] = static_cast<Block>((b[0] >> 3) & std::byte{ 0b111 });
				break;
			case 2:
				blocks[i] = static_cast<Block>(((b[0] >> 5) & std::byte{ 0b110 }) | (b[1] & std::byte{ 0b001 }));
				break;
			case 3:
				blocks[i] = static_cast<Block>((b[1] >> 1) & std::byte{ 0b111 });
				break;
			case 4:
				blocks[i] = static_cast<Block>((b[1] >> 4) & std::byte{ 0b111 });
				break;
			case 5:
				blocks[i] = static_cast<Block>(((b[1] >> 5) & std::byte{ 0b100 }) | (b[2] & std::byte{ 0b011 }));
				break;
			case 6:
				blocks[i] = static_cast<Block>((b[2] >> 2) & std::byte{ 0b111 });
				break;
			case 7:
				blocks[i] = static_cast<Block>((b[2] >> 5) & std::byte{ 0b111 });
				break;
			}
		}

		return grid;
	}

	void CompactGrid::Compact(const Grid& grid)
	{
		Data = std::make_unique_for_overwrite<std::byte[]>(DataLength());

		const Block* blocks = grid.BlocksBegin();

		for (int i = 0; i + 7 < Width * Height; i += 8)
		{
			std::byte* b = &Data[i / 8 * 3];
			b[0] = static_cast<std::byte>(blocks[i]);
			b[0] |= static_cast<std::byte>(blocks[i + 1]) << 3;
			b[0] |= (static_cast<std::byte>(blocks[i + 2]) & std::byte{ 0b110 }) << 5;
			b[1] = static_cast<std::byte>(blocks[i + 2]) & std::byte{ 0b001 };
			b[1] |= static_cast<std::byte>(blocks[i + 3]) << 1;
			b[1] |= static_cast<std::byte>(blocks[i + 4]) << 4;
			b[1] |= (static_cast<std::byte>(blocks[i + 5]) & std::byte{ 0b100 }) << 5;
			b[2] = static_cast<std::byte>(blocks[i + 5]) & std::byte{ 0b011 };
			b[2] |= static_cast<std::byte>(blocks[i + 6]) << 2;
			b[2] |= static_cast<std::byte>(blocks[i + 7]) << 5;
		}

		for (int i = Width * Height - (Width * Height % 8); i < Width * Height; i++)
		{
			std::byte* b = &Data[i / 8 * 3];
			switch (i % 8)
			{
			case 0:
				b[0] = static_cast<std::byte>(blocks[i]);
				break;
			case 1:
				b[0] |= static_cast<std::byte>(blocks[i]) << 3;
				break;
			case 2:
				b[0] |= (static_cast<std::byte>(blocks[i]) & std::byte{ 0b110 }) << 5;
				b[1] = static_cast<std::byte>(blocks[i]) & std::byte{ 0b001 };
				break;
			case 3:
				b[1] |= static_cast<std::byte>(blocks[i]) << 1;
				break;
			case 4:
				b[1] |= static_cast<std::byte>(blocks[i]) << 4;
				break;
			case 5:
				b[1] |= (static_cast<std::byte>(blocks[i]) & std::byte{ 0b100 }) << 5;
				b[2] = static_cast<std::byte>(blocks[i]) & std::byte{ 0b011 };
				break;
			case 6:
				b[2] |= static_cast<std::byte>(blocks[i]) << 2;
				break;
			case 7:
				b[2] |= static_cast<std::byte>(blocks[i]) << 5;
				break;
			}
		}
	}
}