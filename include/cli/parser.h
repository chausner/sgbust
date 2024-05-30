#pragma once

#include <optional>
#include <string>
#include <variant>

struct SolveCLIOptions
{
	std::string GridFile;
	std::string SolutionPrefix;
	std::optional<unsigned int> MaxDBSize = std::nullopt;
	std::optional<unsigned int> MaxDepth = std::nullopt;
	bool DontAddToDBLastDepth = false;
	bool TrimDB = true;
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
	unsigned int SmallestGroupSize;
	std::optional<unsigned int> NumGrids = std::nullopt;
	std::optional<unsigned int> MaxDBSize = std::nullopt;
};

using CLIOptions = std::variant<
	SolveCLIOptions,
	GenerateCLIOptions,
	ShowCLIOptions,
	BenchmarkCLIOptions
>;

std::variant<CLIOptions, int> ParseArgs(int argc, const char* argv[]);