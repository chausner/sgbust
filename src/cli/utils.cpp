#include "cli/utils.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace sgbust::cli
{
#ifdef _WIN32
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
#endif

    std::optional<unsigned int> GetConsoleWidth()
    {
#ifdef _WIN32
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdOut == INVALID_HANDLE_VALUE)
            return std::nullopt;

        CONSOLE_SCREEN_BUFFER_INFO info{};
        if (!GetConsoleScreenBufferInfo(stdOut, &info))
            return std::nullopt;

        return info.srWindow.Right - info.srWindow.Left + 1;
#else
        winsize size{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0)
            return std::nullopt;

        return size.ws_col;
#endif
    }
}