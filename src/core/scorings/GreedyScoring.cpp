#include "core/scorings/GreedyScoring.h"

#include <limits>
#include <utility>

namespace sgbust
{
    GreedyScoring::GreedyScoring(GroupSizeFunc groupScore, int clearanceBonus, LeftoverPenaltyFunc leftoverPenalty)
        : groupScore(std::move(groupScore)), clearanceBonus(clearanceBonus), leftoverPenalty(std::move(leftoverPenalty)) {}

    Score GreedyScoring::CreateScore(const Grid& grid, unsigned int minGroupSize) const
    {
        int score = 0;

        bool hasGroups = grid.HasGroups(minGroupSize);

        if (clearanceBonus != 0 && grid.IsEmpty())
            score -= clearanceBonus;
        if (leftoverPenalty != nullptr && !hasGroups)
            score += leftoverPenalty(grid.GetNumberOfBlocks());

        if (hasGroups)
            return Score(score);
        else
            return Score(score, std::numeric_limits<float>::quiet_NaN());
    }

    Score GreedyScoring::RemoveGroup(const Score& oldScore, const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int minGroupSize) const
    {
        int newScore = oldScore.Value - groupScore(group.size());

        bool hasGroups = newGrid.HasGroups(minGroupSize);

        if (clearanceBonus != 0 && newGrid.IsEmpty())
            newScore -= clearanceBonus;
        if (leftoverPenalty != nullptr && !hasGroups)
            newScore += leftoverPenalty(newGrid.GetNumberOfBlocks());

        if (hasGroups)
            return Score(newScore);
        else
            return Score(newScore, std::numeric_limits<float>::quiet_NaN());
    }

    bool GreedyScoring::IsPerfectScore(const Score& score) const
    {
        return false;
    }
}