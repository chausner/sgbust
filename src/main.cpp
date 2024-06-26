#include <exception>
#include <iostream>
#include <utility>
#include <variant>

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

        std::variant<CLIOptions, int> parsedArgs = ParseArgs(argc, argv);

        if (std::holds_alternative<int>(parsedArgs))
            return std::get<int>(parsedArgs);
        
        CLIOptions cliOptions = std::get<CLIOptions>(std::move(parsedArgs));

        std::visit([](auto&& arg) { RunCommand(arg); }, cliOptions);

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