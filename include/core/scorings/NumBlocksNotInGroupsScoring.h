#pragma once

#include "core/Scoring.h"

namespace sgbust
{
    class NumBlocksNotInGroupsScoring : public Scoring
    {
    public:
        Score CreateScore(const Grid& grid, unsigned int minGroupSize) const override;
        Score RemoveGroup(const Score& oldScore, const Grid& oldGrid, const Group& group, const Grid& newGrid, unsigned int minGroupSize) const override;
        bool IsPerfectScore(const Score& score) const override;
    };
}