#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace sgbust
{
    struct Solution
    {
        Solution() = default;
        Solution(std::string_view string);
        Solution(const Solution& solution);
        Solution(Solution&& solution) noexcept = default;
        Solution& operator=(const Solution& solution);
        Solution& operator=(Solution&& solution) noexcept = default;

        Solution Append(unsigned char step) const;
        Solution Append(const Solution& solution) const;
        std::string AsString() const;
        std::span<const unsigned char> Steps() const { return std::span(steps.get(), steps.get() + GetLength()); }
        unsigned int GetLength() const;
        bool IsEmpty() const { return steps == nullptr; }

        unsigned char operator[](unsigned int index) const { return steps[index]; }

    private:
        std::unique_ptr<unsigned char[]> steps;
    };
}