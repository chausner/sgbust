#pragma once

#include <cstddef>
#include <optional>

namespace sgbust
{
	std::optional<size_t> GetCurrentMemoryUsage();
}