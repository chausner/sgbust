#include <functional>
#include <numeric>
#include "Solver.h"
#include "Scoring.h"

namespace bloc
{
	//Scoring::Scoring(const Grid& grid, unsigned int smallestGroupSize)
	//{
	//	Score = grid.GetNumberOfBlocks();
	//}
	//
	//Scoring Scoring::RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const
	//{
	//	return Score - group.size();
	//}
	//
	//bool Scoring::IsPerfect() const
	//{
	//	return Score == 0;
	//}


	//Scoring::Scoring(const Grid& grid, unsigned int smallestGroupSize) : Score(0)
	//{
	//}
	//
	//Scoring Scoring::RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const
	//{
	//	return Score - group.size() * (group.size() - 1);
	//}
	//
	//bool Scoring::IsPerfect() const
	//{
	//	return false;
	//}


	//Scoring::Scoring(const Grid& grid, unsigned int smallestGroupSize) : Score(0)
	//{
	//}
	//
	//Scoring Scoring::RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const
	//{
	//	return Score - ((group.size() - 2) * (group.size() - 2) + group.size());
	//}
	//
	//bool Scoring::IsPerfect() const
	//{
	//	return false;
	//}

	//Scoring Scoring::RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const

	Scoring::Scoring(const Grid& grid, unsigned int smallestGroupSize)
	{
		unsigned int numberOfBlocks = grid.GetNumberOfBlocks();
		static thread_local std::vector<std::vector<Position>> groups;
		grid.GetGroups(groups, smallestGroupSize);
		unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });

		Score = numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5);
	}

	Scoring Scoring::RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const
	{
		unsigned int numberOfBlocks = newGrid.GetNumberOfBlocks();
		static thread_local std::vector<std::vector<Position>> groups;
		newGrid.GetGroups(groups, smallestGroupSize);
		unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });

		return numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5);
	}

	bool Scoring::IsPerfect() const
	{
		return Score == 0;
	}


	//Scoring::Scoring(const Grid& grid, unsigned int smallestGroupSize)
	//{
	//	unsigned int numberOfBlocks = grid.GetNumberOfBlocks();
	//	static thread_local std::vector<std::vector<Position>> groups;
	//	grid.GetGroups(groups, smallestGroupSize);
	//	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });
	//	unsigned int numberOfNoneBlocks = grid.Width * grid.Height - numberOfBlocks;
	//
	//	Score = numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5) + numberOfNoneBlocks * 0.25;
	//}
	//
	//Scoring Scoring::RemoveGroup(const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int smallestGroupSize) const
	//{
	//	unsigned int numberOfBlocks = newGrid.GetNumberOfBlocks();
	//	static thread_local std::vector<std::vector<Position>> groups;
	//	newGrid.GetGroups(groups, smallestGroupSize);
	//	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });
	//	unsigned int numberOfNoneBlocks = newGrid.Width * newGrid.Height - numberOfBlocks;
	//
	//	return numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5) + numberOfNoneBlocks * 0.25;
	//}
	//
	//bool Scoring::IsPerfect() const
	//{
	//	return Score == 0;
	//}
}