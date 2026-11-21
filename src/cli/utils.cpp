#include "cli/utils.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace sgbust::cli
{
    bool EnableVTMode()
    {
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdOut == INVALID_HANDLE_VALUE)
            return false;

        DWORD mode = 0;
        if (!GetConsoleMode(stdOut, &mode))
            return false;

        if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0)
        {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            if (!SetConsoleMode(stdOut, mode))
                return false;
        }

        return true;
    }

    std::optional<unsigned int> GetConsoleWidth()
    {
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdOut == INVALID_HANDLE_VALUE)
            return std::nullopt;

        CONSOLE_SCREEN_BUFFER_INFO info{};
        if (!GetConsoleScreenBufferInfo(stdOut, &info))
            return std::nullopt;

        return info.srWindow.Right - info.srWindow.Left + 1;
    }
}

#elif defined(__linux__)

#include <sys/ioctl.h>
#include <unistd.h>

namespace sgbust::cli
{
    std::optional<unsigned int> GetConsoleWidth()
    {
        winsize size{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0)
            return std::nullopt;

        return size.ws_col;
    }
}

#else

namespace sgbust::cli
{
    std::optional<unsigned int> GetConsoleWidth()
    {
        return std::nullopt;
    }
}

#endif
