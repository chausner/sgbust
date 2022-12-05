#pragma once

#include <numeric>
#include <vector>
#include "BlockGrid.h"

struct Score
{
	int Value;

	explicit Score(int value) : Value(value) {}

	bool operator<(const Score& other) const
	{
		return Value < other.Value;
	}
};

class Scoring
{
public:
	virtual Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const = 0;
	virtual Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const = 0;
	virtual bool IsPerfectScore(const Score& score) const = 0;
};

class NScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(blockGrid.GetNumberOfBlocks());
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(oldScore.Value - group.size());
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return score.Value == 0;
	}
};

class NNminusOneScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(0);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(oldScore.Value - group.size() * (group.size() - 1));
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};

class NminusTwoSquaredScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(0);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		if (group.size() <= 2)
			return oldScore;
		else
			return Score(oldScore.Value - (group.size() - 2) * (group.size() - 2));
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};

class NminusTwoSquaredPlusNScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(0);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		if (group.size() <= 2)
			return oldScore;
		else
			return Score(oldScore.Value - ((group.size() - 2) * (group.size() - 2) + group.size()));
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};

class NumBlocksNotInGroupsScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		std::vector<std::vector<Position>> groups;
		blockGrid.GetGroups(groups, smallestGroupSize);

		int numBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [](const auto& group) { return group.size(); });
		int numBlocksNotInGroups = blockGrid.GetNumberOfBlocks() - numBlocksInGroups;
		return Score(numBlocksNotInGroups);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		std::vector<std::vector<Position>> groups;
		newBlockGrid.GetGroups(groups, smallestGroupSize);

		int numBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [](const auto& group) { return group.size(); });
		int numBlocksNotInGroups = newBlockGrid.GetNumberOfBlocks() - numBlocksInGroups;
		return Score(numBlocksNotInGroups);
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};