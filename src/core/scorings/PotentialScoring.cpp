#include "core/scorings/PotentialScoring.h"

#include <functional>
#include <numeric>
#include <utility>

namespace sgbust
{
    PotentialScoring::PotentialScoring(GroupSizeFunc groupScore, int clearanceBonus, LeftoverPenaltyFunc leftoverPenalty)
        : groupScore(std::move(groupScore)), clearanceBonus(clearanceBonus), leftoverPenalty(std::move(leftoverPenalty)) {}

    Score PotentialScoring::CreateScore(const Grid& grid, unsigned int minGroupSize) const
    {
        static thread_local std::vector<Group> groups;
        grid.GetGroups(groups, minGroupSize);

        int score = 0;

        if (clearanceBonus != 0 && grid.IsEmpty())
            score -= clearanceBonus;
        if (leftoverPenalty != nullptr && groups.empty())
            score += leftoverPenalty(grid.GetNumberOfBlocks());

        int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return groupScore(group.size()); });

        return Score(score, score - potentialGroupsScore);
    }

    Score PotentialScoring::RemoveGroup(const Score& oldScore, const Grid& oldGrid, const Group& group, const Grid& newGrid, unsigned int minGroupSize) const
    {
        static thread_local std::vector<Group> groups;
        newGrid.GetGroups(groups, minGroupSize);

        int newScore = oldScore.Value - groupScore(group.size());

        if (clearanceBonus != 0 && newGrid.IsEmpty())
            newScore -= clearanceBonus;
        if (leftoverPenalty != nullptr && groups.empty())
            newScore += leftoverPenalty(newGrid.GetNumberOfBlocks());
        
        int potentialGroupsScore = std::transform_reduce(groups.begin(), groups.end(), 0, std::plus<>(), [this](const auto& group) { return groupScore(group.size()); });

        return Score(newScore, newScore - potentialGroupsScore);
    }

    bool PotentialScoring::IsPerfectScore(const Score& score) const
    {
        return false;
    }
}