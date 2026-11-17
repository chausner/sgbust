#include <cstdio>
#include <exception>
#include <expected>
#include <print>
#include <utility>

#include "cli/parser.h"
#include "cli/commands.h"
#include "cli/utils.h"
#include "mimalloc-new-delete.h"

int main(int argc, const char* argv[])
{
    try
    {
#ifdef _WIN32
        EnableVTMode();
#endif

        std::expected<CLIOptions, int> parsedArgs = ParseArgs(argc, argv);

        if (!parsedArgs.has_value())
            return parsedArgs.error();     

        std::visit([](auto&& arg) { RunCommand(arg); }, *parsedArgs);

        return 0;
    }
    catch (const std::exception& ex)
    {
        std::println(stderr, "Exception: {}", ex.what());
        return 1;
    }
    catch (...)
    {
        std::println(stderr, "An unknown exception occurred");
        return 1;
    }
}