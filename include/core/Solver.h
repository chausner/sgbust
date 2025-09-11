#pragma once

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <shared_mutex>

#include "core/CompactGrid.h"
#include "core/Grid.h"
#include "core/Scoring.h"
#include "mimalloc.h"
#include "parallel_hashmap/phmap.h"
#include "wyhash.h"

template <>
class std::hash<sgbust::CompactGrid>
{
public:
    std::size_t operator()(const sgbust::CompactGrid& key) const
    {
        return wyhash(key.Data.get(), key.DataLength(), 0, _wyp);
    }
};

template <>
class std::equal_to<sgbust::CompactGrid>
{
public:
    constexpr bool operator()(const sgbust::CompactGrid& lhs, const sgbust::CompactGrid& rhs) const
    {
        return lhs.Width == rhs.Width &&
            lhs.Height == rhs.Height &&
            std::equal(lhs.Data.get(), lhs.Data.get() + lhs.DataLength(), rhs.Data.get());
    }
};

namespace sgbust
{
    using GridHashSet = phmap::parallel_flat_hash_set<CompactGrid, std::hash<CompactGrid>, std::equal_to<CompactGrid>, mi_stl_allocator<CompactGrid>, 6, std::mutex>;

    struct SolverResult
    {
        int BestScore;
        Solution BestSolution;
        Grid SolutionGrid;
    };

    class Solver
    {
        unsigned int minGroupSize = 0;
        const Scoring* scoring = nullptr;
        unsigned int depth = 0;
        std::map<Score, GridHashSet> grids;
        Solution solutionPrefix;
        Solution solution;
        std::optional<Grid> solutionGrid;
        std::optional<int> bestScore;
        unsigned int beamSize = 0;
        double multiplier = 0;
        mutable std::shared_mutex mutex;

        void SolveDepth(bool maxDepthReached, bool& stop);
        unsigned int SolveGrid(const Grid& grid, Score score, std::map<Score, GridHashSet>& newGrids, bool maxDepthReached, bool& stop);
        void CheckSolution(const Grid& grid, Score score, bool& stop);
        void PrintStats() const;
        void PrintProgress(const std::map<Score, GridHashSet>& newGrids, unsigned int gridsSolved, unsigned int newBeamSize) const;
		void ClearProgress() const;
        void TrimBeam();

    public:
        std::optional<unsigned int> MaxBeamSize = std::nullopt;
        std::optional<unsigned int> MaxDepth = std::nullopt;
        bool TrimmingEnabled = true;
        double TrimmingSafetyFactor = 1.25;
        bool Quiet = false;

        std::optional<SolverResult> Solve(const Grid& grid, unsigned int minGroupSize, const Scoring& scoring, const Solution& solutionPrefix = {});
    };
}