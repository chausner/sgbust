#include "core/MemoryUsage.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>

namespace sgbust
{
    std::optional<std::size_t> GetCurrentMemoryUsage()
    {
        PROCESS_MEMORY_COUNTERS counters;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) == 0)
            return std::nullopt;
        return counters.WorkingSetSize;
    }
}

#elif defined(__linux__)

#include <fstream>
#include <unistd.h>

namespace sgbust
{
    std::optional<std::size_t> GetCurrentMemoryUsage()
    {
        std::ifstream file("/proc/self/statm");
        std::size_t size, resident, share;
        file >> size >> resident >> share;
        if (file.fail())
            return std::nullopt;

        long pageSize = sysconf(_SC_PAGESIZE);
        if (pageSize == -1)
            return std::nullopt;

        return resident * pageSize;
    }
}

#else

namespace sgbust
{
    std::optional<std::size_t> GetCurrentMemoryUsage()
    {
        return std::nullopt;
    }
}

#endif