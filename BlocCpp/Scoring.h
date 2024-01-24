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
	virtual ~Scoring() = default;

	virtual Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const = 0;
	virtual Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const = 0;
	virtual bool IsPerfectScore(const Score& score) const = 0;
};

using GroupSizeFunc = std::function<int(unsigned int groupSize)>;
using LeftoverPenaltyFunc = std::function<int(unsigned int numBlocksRemaining)>;

class GreedyScoring : public Scoring
{
	GroupSizeFunc groupScore;
	int clearanceBonus;
	LeftoverPenaltyFunc leftoverPenalty;

public:
	GreedyScoring(GroupSizeFunc groupScore, int clearanceBonus = 0, LeftoverPenaltyFunc leftoverPenalty = nullptr)
	    : groupScore(std::move(groupScore)), clearanceBonus(clearanceBonus), leftoverPenalty(std::move(leftoverPenalty)) {}

	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		int score = 0;

		if (clearanceBonus != 0 && blockGrid.IsEmpty())
			score -= clearanceBonus;
		if (leftoverPenalty != nullptr && !blockGrid.HasGroups(smallestGroupSize))
			score += leftoverPenalty(blockGrid.GetNumberOfBlocks());

		return Score(score);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		int newScore = oldScore.Value - groupScore(group.size());

		if (clearanceBonus != 0 && newBlockGrid.IsEmpty())
			newScore -= clearanceBonus;
		if (leftoverPenalty != nullptr && !newBlockGrid.HasGroups(smallestGroupSize))
			newScore += leftoverPenalty(newBlockGrid.GetNumberOfBlocks());

		return Score(newScore);
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
		return CreateScore(newBlockGrid, smallestGroupSize);
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};

class PotentialScoring : public Scoring
{
	GroupSizeFunc groupScore;
	int clearanceBonus;
	LeftoverPenaltyFunc leftoverPenalty;

	const float constant = std::atof(std::getenv("constant"));

public:
	PotentialScoring(GroupSizeFunc groupScore, int clearanceBonus = 0, LeftoverPenaltyFunc leftoverPenalty = nullptr)
	    : groupScore(std::move(groupScore)), clearanceBonus(clearanceBonus), leftoverPenalty(std::move(leftoverPenalty)) {}

	Score CreateScore(const BlockGrid& blockGrid, unsigned int smallestGroupSize) const override
	{
		static thread_local std::vector<std::vector<Position>> groups;
		blockGrid.GetGroups(groups, smallestGroupSize);

		int score = 0;

		if (clearanceBonus != 0 && blockGrid.IsEmpty())
			score -= clearanceBonus;
		if (leftoverPenalty != nullptr && groups.empty())
			score += leftoverPenalty(blockGrid.GetNumberOfBlocks());

		int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return groupScore(group.size()); });
		return Score(score, -potentialGroupsScore);
	}

	Score RemoveGroup(const Score& oldScore, const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const override
	{
		static thread_local std::vector<std::vector<Position>> groups;
		newBlockGrid.GetGroups(groups, smallestGroupSize);

		int newScore = oldScore.Value - groupScore(group.size());

		if (clearanceBonus != 0 && newBlockGrid.IsEmpty())
			newScore -= clearanceBonus;
		if (leftoverPenalty != nullptr && groups.empty())
			newScore += leftoverPenalty(newBlockGrid.GetNumberOfBlocks());

		int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return groupScore(group.size()); });

		// TODO: should clearanceBonus and leftoverPenalty be included in newObjective or not?

		float newObjective = newScore * 0.99f + -potentialGroupsScore + constant;

		return Score(newScore, newObjective);
	}

	bool IsPerfectScore(const Score& score) const override
	{
		return false;
	}
};