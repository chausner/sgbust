#pragma once

#include <functional>
#include <numeric>
#include <utility>
#include <vector>
#include "BlockGrid.h"

struct Score
{
	int Value;
	float Objective;

	explicit Score(int valueAndObjective) : Value(valueAndObjective), Objective(valueAndObjective) {}
	Score(int value, float objective) : Value(value), Objective(objective) {}

	bool operator<(const Score& other) const
	{
		return Objective < other.Objective || (Objective == other.Objective && Value < other.Value);
	}
};

class Scoring
{
public:
	virtual Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const = 0;
	virtual Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const = 0;
	virtual bool IsPerfectScore(const Score& score) const = 0;
};

class SimpleScoring : public Scoring
{
	std::function<int (unsigned int groupSize)> baseScoring;

public:
	SimpleScoring(std::function<int (unsigned int groupSize)> baseScoring) : baseScoring(std::move(baseScoring)) {}

	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(0);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		return Score(oldScore.Value - baseScoring(group.size()));
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
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

class NNminusOneScoring : public SimpleScoring
{
public:
	NNminusOneScoring() : SimpleScoring([](unsigned int groupSize) { return groupSize * (groupSize - 1); }) {}
};

class NminusTwoSquaredScoring : public SimpleScoring
{
public:
	NminusTwoSquaredScoring() : SimpleScoring([](unsigned int groupSize) { return groupSize > 2 ? (groupSize - 2) * (groupSize - 2) : 0; }) {}
};

class NminusTwoSquaredPlusNScoring : public SimpleScoring
{
	static constexpr auto scoreFunc = [](unsigned int groupSize) { return groupSize > 2 ? (groupSize - 2) * (groupSize - 2) + groupSize : 0; };

public:
	NminusTwoSquaredPlusNScoring() : SimpleScoring(scoreFunc) {}
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

class PotentialGroupsScoreScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		static thread_local std::vector<std::vector<Position>> groups;
		blockGrid.GetGroups(groups, smallestGroupSize);

		auto score = [](int groupSize) { return (groupSize - 2) * (groupSize - 2) + groupSize; };

		int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [score](const auto& group) { return score(group.size()); });
		return Score(0, -potentialGroupsScore);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		auto score = [](int groupSize) { return (groupSize - 2) * (groupSize - 2) + groupSize; };

		int newScore = oldScore.Value - score(group.size());
		float newObjective = newScore * 0.99f + CreateScore(newBlockGrid, smallestGroupSize).Objective;

		return Score(newScore, newObjective);
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};

class PotentialNScoring : public Scoring
{
public:
	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		static thread_local std::vector<std::vector<Position>> groups;
		blockGrid.GetGroups(groups, smallestGroupSize);

		auto score = [](int groupSize) { return groupSize; };

		int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [score](const auto& group) { return score(group.size()); });
		return Score(0, -potentialGroupsScore);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		auto score = [](int groupSize) { return groupSize; };

		int newScore = oldScore.Value - score(group.size());
		float newObjective = newScore * 0.99f + CreateScore(newBlockGrid, smallestGroupSize).Objective;

		return Score(newScore, newObjective);
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};

class PotentialScoreMetaScoring : public Scoring
{
	std::function<int (unsigned int groupSize)> baseScoring;

public:
	PotentialScoreMetaScoring(std::function<int (unsigned int groupSize)> baseScoring) : baseScoring(std::move(baseScoring)) {}

	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		static thread_local std::vector<std::vector<Position>> groups;
		blockGrid.GetGroups(groups, smallestGroupSize);

		int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return baseScoring(group.size()); });
		return Score(0, -potentialGroupsScore);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		int newScore = oldScore.Value - baseScoring(group.size());
		float newObjective = newScore + CreateScore(newBlockGrid, smallestGroupSize).Objective;

		return Score(newScore, newObjective);
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};