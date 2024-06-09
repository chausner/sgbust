#pragma once

#include "core/Scoring.h"

namespace sgbust
{
    class PotentialScoring : public Scoring
    {
        GroupSizeFunc groupScore;
        int clearanceBonus;
        LeftoverPenaltyFunc leftoverPenalty;

    public:
        PotentialScoring(GroupSizeFunc groupScore, int clearanceBonus = 0, LeftoverPenaltyFunc leftoverPenalty = nullptr);
        Score CreateScore(const Grid& grid, unsigned int minGroupSize) const override;
        Score RemoveGroup(const Score& oldScore, const Grid& oldGrid, const std::vector<Position>& group, const Grid& newGrid, unsigned int minGroupSize) const override;
        bool IsPerfectScore(const Score& score) const override;
    };
}