#include "core/scorings/PotentialScoring.h"

#include <functional>
#include <limits>
#include <numeric>
#include <utility>

namespace sgbust
{
    PotentialScoring::PotentialScoring(GroupSizeFunc groupScore, int clearanceBonus, LeftoverPenaltyFunc leftoverPenalty)
        : groupScore(std::move(groupScore)), clearanceBonus(clearanceBonus), leftoverPenalty(std::move(leftoverPenalty)) {}

    Score PotentialScoring::CreateScore(const Grid& grid, unsigned int minGroupSize) const
    {
        static thread_local std::vector<std::vector<Position>> groups;
        grid.GetGroups(groups, minGroupSize);

        int score = 0;

        if (clearanceBonus != 0 && grid.IsEmpty())
            score -= clearanceBonus;
        if (leftoverPenalty != nullptr && groups.empty())
            score += leftoverPenalty(grid.GetNumberOfBlocks());

        int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return groupScore(group.size()); });
        return Score(score, -potentialGroupsScore);
    }

    Score PotentialScoring::RemoveGroup(const Score& oldScore, const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int minGroupSize) const
    {
        static thread_local std::vector<std::vector<Position>> groups;
        newGrid.GetGroups(groups, minGroupSize);

        int newScore = oldScore.Value - groupScore(group.size());

        if (clearanceBonus != 0 && newGrid.IsEmpty())
            newScore -= clearanceBonus;
        if (leftoverPenalty != nullptr && groups.empty())
            newScore += leftoverPenalty(newGrid.GetNumberOfBlocks());

        float newObjective;
        if (!groups.empty())
        {
            int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return groupScore(group.size()); });

            newObjective = newScore * 0.99f + -potentialGroupsScore;
        }
        else
            newObjective = std::numeric_limits<float>::quiet_NaN();

        return Score(newScore, newObjective);
    }

    bool PotentialScoring::IsPerfectScore(const Score& score) const
    {
        return false;
    }
}