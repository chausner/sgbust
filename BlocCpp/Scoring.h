#pragma once

#include <vector>
#include "BlockGrid.h"

struct Scoring
{
	int Score;

	Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize);
	Scoring(int score) : Score(score) {}
	Scoring RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize);
	bool IsPerfect() const;

	bool operator<(const Scoring& other) const
	{
		return Score < other.Score;
	}
};