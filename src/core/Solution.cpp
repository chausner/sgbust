#include "core/Solution.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace sgbust
{
    Solution::Solution(std::string_view string)
    {
        if (string.empty())
            return;

        std::vector<unsigned char> steps;
        steps.reserve(string.length());

        for (auto it = string.begin(); it != string.end(); )
        {
            auto isAToZ = [](char c) { return c >= 'A' && c <= 'Z'; };

            if (isAToZ(it[0]))
            {
                steps.push_back(it[0] - 65);
                it++;
            }
            else if (it[0] == '(' && std::distance(it, string.end()) >= 4 && isAToZ(it[1]) && isAToZ(it[2]) && it[3] == ')')
            {
                unsigned int n = (it[1] - 64) * 26 + (it[2] - 65);
                if (n > 255)
                    throw std::invalid_argument("Invalid solution string");
                steps.push_back(n);
                it += 4;
            }
            else
                throw std::invalid_argument("Invalid solution string");
        }

        this->steps = std::make_unique_for_overwrite<unsigned char[]>(steps.size() + 1);
        std::copy(steps.begin(), steps.end(), this->steps.get());
        this->steps.get()[steps.size()] = 0xFF;
    }

    Solution::Solution(const Solution& solution)
    {
        if (solution.steps != nullptr)
        {
            int length = solution.GetLength();
            steps = std::make_unique_for_overwrite<unsigned char[]>(length + 1);
            std::copy(solution.steps.get(), solution.steps.get() + length + 1, steps.get());
        }
    }

    Solution& Solution::operator=(const Solution& solution)
    {
        if (solution.steps != nullptr)
        {
            int length = solution.GetLength();
            steps = std::make_unique_for_overwrite<unsigned char[]>(length + 1);
            std::copy(solution.steps.get(), solution.steps.get() + length + 1, steps.get());
        }
        else
            steps = nullptr;

        return *this;
    }

    Solution Solution::Append(unsigned char step) const
    {
        Solution result;

        if (steps != nullptr)
        {
            int length = GetLength();
            result.steps = std::make_unique_for_overwrite<unsigned char[]>(length + 2);
            std::copy(steps.get(), steps.get() + length, result.steps.get());
            result.steps[length] = step;
            result.steps[length + 1] = 0xFF;
        }
        else
        {
            result.steps = std::make_unique_for_overwrite<unsigned char[]>(2);
            result.steps[0] = step;
            result.steps[1] = 0xFF;
        }

        return result;
    }

    Solution Solution::Append(const Solution& solution) const
    {
        if (solution.IsEmpty())
            return *this;
        else if (IsEmpty())
            return solution;
        else
        {
            Solution result;
            unsigned int length1 = GetLength();
            unsigned int length2 = solution.GetLength();
            result.steps = std::make_unique_for_overwrite<unsigned char[]>(length1 + length2 + 1);
            std::copy(steps.get(), steps.get() + length1, result.steps.get());
            std::copy(solution.steps.get(), solution.steps.get() + length2, result.steps.get() + length1);
            result.steps[length1 + length2] = 0xFF;
            return result;
        }
    }

    std::string Solution::AsString() const
    {
        if (steps == nullptr)
            return std::string();

        std::string solution;

        for (unsigned char* step = steps.get(); *step != 0xFF; step++)
        {
            if (*step < 26)
                solution.push_back((char)(*step + 65));
            else
            {
                solution.push_back('(');
                solution.push_back((char)((*step / 26) + 64));
                solution.push_back((char)((*step % 26) + 65));
                solution.push_back(')');
            }
        }

        return solution;
    }

    unsigned int Solution::GetLength() const
    {
        if (steps == nullptr)
            return 0;

        int length = 0;

        while (steps[length] != 0xFF)
            length++;

        return length;
    }
}