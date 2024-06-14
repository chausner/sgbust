#pragma once

#include <functional>
#include <vector>

#include "core/Grid.h"

namespace sgbust
{
    struct Score
    {
        int Value;
        float Objective;

        explicit Score(int valueAndObjective) : Value(valueAndObjective), Objective(valueAndObjective) {}
        Score(int value, float objective) : Value(value), Objective(objective) {}

        bool operator<(const Score& other) const
        {
            return Objective < other.Objective || (Objective == other.Objective && Value < other.Value);
        }
    };

    class Scoring
    {
    public:
        virtual ~Scoring() = default;

        virtual Score CreateScore(const Grid& grid, unsigned int minGroupSize) const = 0;
        virtual Score RemoveGroup(const Score& oldScore, const Grid& oldGrid, const Group& group, const Grid& newGrid, unsigned int minGroupSize) const = 0;
        virtual bool IsPerfectScore(const Score& score) const = 0;
    };

    using GroupSizeFunc = std::function<int(unsigned int groupSize)>;
    using LeftoverPenaltyFunc = std::function<int(unsigned int numBlocksRemaining)>;
}