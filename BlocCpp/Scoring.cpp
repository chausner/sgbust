#include <functional>
#include <numeric>
#include "BlocSolver.h"
#include "Scoring.h"

//Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize)
//{
//	Score = blockGrid.GetNumberOfBlocks();
//}
//
//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const
//{
//	return Score - group.size();
//}
//
//bool Scoring::IsPerfect() const
//{
//	return Score == 0;
//}


//Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize) : Score(0)
//{
//}
//
//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const
//{
//	return Score - group.size() * (group.size() - 1);
//}
//
//bool Scoring::IsPerfect() const
//{
//	return false;
//}


//Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize) : Score(0)
//{
//}
//
//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const
//{
//	return Score - ((group.size() - 2) * (group.size() - 2) + group.size());
//}
//
//bool Scoring::IsPerfect() const
//{
//	return false;
//}

//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const

Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize)
{
	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();
	static thread_local std::vector<std::vector<Position>> groups;
	blockGrid.GetGroups(groups, smallestGroupSize);
	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });

	Score = numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5);
}

Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const
{
	unsigned int numberOfBlocks = newBlockGrid.GetNumberOfBlocks();
	static thread_local std::vector<std::vector<Position>> groups;
	newBlockGrid.GetGroups(groups, smallestGroupSize);
	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });

	return numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5);
}

bool Scoring::IsPerfect() const
{
	return Score == 0;
}


//Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize)
//{
//	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();
//	static thread_local std::vector<std::vector<Position>> groups;
//	blockGrid.GetGroups(groups, smallestGroupSize);
//	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });
//	unsigned int numberOfNoneBlocks = blockGrid.Width * blockGrid.Height - numberOfBlocks;
//
//	Score = numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5) + numberOfNoneBlocks * 0.25;
//}
//
//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize) const
//{
//	unsigned int numberOfBlocks = newBlockGrid.GetNumberOfBlocks();
//	static thread_local std::vector<std::vector<Position>> groups;
//	newBlockGrid.GetGroups(groups, smallestGroupSize);
//	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });
//	unsigned int numberOfNoneBlocks = newBlockGrid.Width * newBlockGrid.Height - numberOfBlocks;
//
//	return numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5) + numberOfNoneBlocks * 0.25;
//}
//
//bool Scoring::IsPerfect() const
//{
//	return Score == 0;
//}