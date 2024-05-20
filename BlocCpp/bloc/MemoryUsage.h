#pragma once

#include <cstddef>
#include <optional>

namespace bloc
{
	std::optional<size_t> GetCurrentMemoryUsage();
}