#include <exception>
#include <iostream>
#include <variant>

#include "cli/parser.h"
#include "cli/commands.h"
#include "mimalloc-new-delete.h"

int main(int argc, const char* argv[])
{
	try
	{
		std::variant<CLIOptions, int> parsedArgs = ParseArgs(argc, argv);

		if (std::holds_alternative<int>(parsedArgs))
			return std::get<int>(parsedArgs);
		
		CLIOptions cliOptions = std::get<CLIOptions>(parsedArgs);

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