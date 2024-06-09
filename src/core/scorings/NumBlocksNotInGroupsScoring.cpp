#include "core/scorings/NumBlocksNotInGroupsScoring.h"

#include <limits>
#include <numeric>

namespace sgbust
{
    Score NumBlocksNotInGroupsScoring::CreateScore(const Grid& grid, unsigned int minGroupSize) const
    {
        static thread_local std::vector<std::vector<Position>> groups;
        grid.GetGroups(groups, minGroupSize);

        int numBlocksInGroups = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [](const auto& group) { return group.size(); });
        int numBlocksNotInGroups = grid.GetNumberOfBlocks() - numBlocksInGroups;

        if (!groups.empty())
            return Score(numBlocksNotInGroups);
        else
            return Score(numBlocksNotInGroups, std::numeric_limits<float>::quiet_NaN());
    }

    Score NumBlocksNotInGroupsScoring::RemoveGroup(const Score& oldScore, const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int minGroupSize) const
    {
        return CreateScore(newGrid, minGroupSize);
    }

    bool NumBlocksNotInGroupsScoring::IsPerfectScore(const Score& score) const
    {
        return false;
    }
}