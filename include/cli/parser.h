#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "core/Polynom.h"
#include "core/Scoring.h"

enum class ScoringType
{
    Greedy,
    Potential,
    NumBlocksNotInGroups
};

struct ScoringOptions
{
    ::ScoringType ScoringType = ScoringType::Greedy;
    std::optional<sgbust::Polynom> ScoringGroupScore;
    std::optional<int> ScoringClearanceBonus;
    std::optional<sgbust::Polynom> ScoringLeftoverPenalty;
    std::unique_ptr<sgbust::Scoring> Scoring;
};

struct SolveCLIOptions
{
    std::string GridFile;
    ::ScoringOptions ScoringOptions;
    std::string SolutionPrefix;
    std::optional<unsigned int> MaxBeamSize = std::nullopt;
    std::optional<unsigned int> MaxDepth = std::nullopt;
    std::optional<unsigned int> NumStepsToClear = std::nullopt;
    bool TrimmingEnabled = true;
    double TrimmingSafetyFactor = 1.25;
    bool Quiet = false;
};

struct GenerateCLIOptions
{
    std::string GridFile;
    std::optional<unsigned long long> Seed;
    unsigned char Width;
    unsigned char Height;
    unsigned int NumColors;
    unsigned int MinGroupSize;
    bool Quiet = false;
};

struct ShowCLIOptions
{
    std::string GridFile;
    std::string Solution;
};

struct BenchmarkCLIOptions
{
    std::optional<unsigned long long> Seed;
    unsigned char Width;
    unsigned char Height;
    unsigned int NumColors;
    unsigned int MinGroupSize;
    std::optional<unsigned int> NumGrids = std::nullopt;
    ::ScoringOptions ScoringOptions;
    std::optional<unsigned int> MaxBeamSize = std::nullopt;
};

using CLIOptions = std::variant<
    SolveCLIOptions,
    GenerateCLIOptions,
    ShowCLIOptions,
    BenchmarkCLIOptions
>;

std::variant<CLIOptions, int> ParseArgs(int argc, const char* argv[]);