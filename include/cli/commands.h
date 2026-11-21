#pragma once

#include "cli/parser.h"

namespace sgbust::cli
{
    void RunCommand(const SolveCLIOptions& cliOptions);
    void RunCommand(const GenerateCLIOptions& cliOptions);
    void RunCommand(const ShowCLIOptions& cliOptions);
    void RunCommand(const BenchmarkCLIOptions& cliOptions);
}