#pragma once

#include <cstddef>
#include <optional>

namespace sgbust
{
    std::optional<std::size_t> GetCurrentMemoryUsage();
    std::optional<std::size_t> GetPeakMemoryUsage();
}