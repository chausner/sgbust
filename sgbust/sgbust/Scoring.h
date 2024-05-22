#pragma once

#include <vector>
#include "Grid.h"

namespace sgbust
{
	struct Scoring
	{
		int Score;

		Scoring(const Grid& grid, unsigned int smallestGroupSize);
		Scoring(int score) : Score(score) {}
		Scoring RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const;
		bool IsPerfect() const;

		bool operator<(const Scoring& other) const
		{
			return Score < other.Score;
		}
	};
}