#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool EnableVTMode()
{
	HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdOut == INVALID_HANDLE_VALUE)
		return false;

	DWORD mode = 0;
	if (!GetConsoleMode(stdOut, &mode))
		return false;

	mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(stdOut, mode))
		return false;

	return true;
}
#endif