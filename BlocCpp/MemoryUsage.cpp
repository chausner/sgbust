#include "MemoryUsage.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>

std::optional<size_t> GetCurrentMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS counters;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) == 0)
		return std::nullopt;
    return counters.WorkingSetSize;
}

#elif defined(__linux__)

#include <fstream>
#include <unistd.h>

std::optional<size_t> GetCurrentMemoryUsage()
{
    std::ifstream file("/proc/self/statm");
    size_t size, resident, share;
    file >> size >> resident >> share;
    if (!file)
        return std::nullopt;

    long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize == -1)
        return std::nullopt;

    return resident * pageSize;
}

#else

std::optional<size_t> GetCurrentMemoryUsage()
{
    return std::nulloptr;
}

#endif