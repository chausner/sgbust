#include "core/scorings/GreedyScoring.h"

#include <utility>

namespace sgbust
{
    GreedyScoring::GreedyScoring(GroupSizeFunc groupScore, int clearanceBonus, LeftoverPenaltyFunc leftoverPenalty)
        : groupScore(std::move(groupScore)), clearanceBonus(clearanceBonus), leftoverPenalty(std::move(leftoverPenalty)) {}

    Score GreedyScoring::CreateScore(const Grid& grid, unsigned int minGroupSize) const
    {
        int score = 0;

        if (clearanceBonus != 0 && grid.IsEmpty())
            score -= clearanceBonus;
        if (leftoverPenalty != nullptr && !grid.HasGroups(minGroupSize))
            score += leftoverPenalty(grid.GetNumberOfBlocks());

        return Score(score);
    }

    Score GreedyScoring::RemoveGroup(const Score& oldScore, const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int minGroupSize) const
    {
        int newScore = oldScore.Value - groupScore(group.size());

        if (clearanceBonus != 0 && newGrid.IsEmpty())
            newScore -= clearanceBonus;
        if (leftoverPenalty != nullptr && !newGrid.HasGroups(minGroupSize))
            newScore += leftoverPenalty(newGrid.GetNumberOfBlocks());

        return Score(newScore);
    }

    bool GreedyScoring::IsPerfectScore(const Score& score) const
    {
        return false;
    }
}