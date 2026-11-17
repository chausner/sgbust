#include <exception>
#include <expected>
#include <iostream>
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
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "An unknown exception occurred";
        return 1;
    }
}