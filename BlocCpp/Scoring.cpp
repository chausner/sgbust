#include <functional>
#include <numeric>
#include "BlocSolver.h"
#include "Scoring.h"

//Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize)
//{
//	auto blocks = blockGrid.Expand();
//	Score = blockGrid.GetNumberOfBlocks(blocks.get());
//}
//
//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize)
//{
//	return Score - group.size();
//
//	/*unsigned int numberOfBlocks = newBlockGrid.GetNumberOfBlocks();
//	auto groups = newBlockGrid.GetGroups(3);
//
//	unsigned int numberOfBlocksInGroups = 0;
//	for (auto& group : groups)
//		numberOfBlocksInGroups += group.size();
//
//	return numberOfBlocks - numberOfBlocksInGroups;*/
//
//	//return Score - group.size() * (group.size() - 1);
//
//	//return Score - ((group.size() - 2) * (group.size() - 2) + group.size());
//}
//
//bool Scoring::IsPerfect() const
//{
//	return Score == 0;
//}

Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize)
{
	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();
	auto groups = blockGrid.GetGroups(smallestGroupSize);
	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });

	Score = numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5);
}

Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize)
{
	unsigned int numberOfBlocks = newBlockGrid.GetNumberOfBlocks();
	auto groups = newBlockGrid.GetGroups(smallestGroupSize);
	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });

	return numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5);
}

bool Scoring::IsPerfect() const
{
	return false;
}

//Scoring::Scoring(const BlockGrid& blockGrid, unsigned int smallestGroupSize)
//{
//	unsigned int numberOfBlocks = blockGrid.GetNumberOfBlocks();
//	auto groups = blockGrid.GetGroups(smallestGroupSize);
//	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });
//	unsigned int numberOfNoneBlocks = blockGrid.Width * blockGrid.Height - numberOfBlocks;
//
//	Score = numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5) + numberOfNoneBlocks * 0.25;
//}
//
//Scoring Scoring::RemoveGroup(const BlockGrid& oldBlockGrid, const std::vector<Position>& group, const BlockGrid& newBlockGrid, unsigned int smallestGroupSize)
//{
//	unsigned int numberOfBlocks = newBlockGrid.GetNumberOfBlocks();
//	auto groups = newBlockGrid.GetGroups(smallestGroupSize);
//	unsigned int numberOfBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0u, std::plus(), [](auto& group) { return group.size(); });
//	unsigned int numberOfNoneBlocks = newBlockGrid.Width * newBlockGrid.Height - numberOfBlocks;
//
//	return numberOfBlocks - (int)(numberOfBlocksInGroups * 0.5) + numberOfNoneBlocks * 0.25;
//}
//
//bool Scoring::IsPerfect() const
//{
//	return false;
//}