#pragma once

#include <optional>

namespace sgbust::cli
{
#ifdef _WIN32
    bool EnableVTMode();
#endif

    std::optional<unsigned int> GetConsoleWidth();
}