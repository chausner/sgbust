#include "core/Polynomial.h"

#include <cmath>
#include <ranges>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sgbust
{
    Polynomial::Polynomial(std::initializer_list<int> coefficients) : 
        coefficients(coefficients)
    {
    }

    Polynomial::Polynomial(std::string_view str)
    {
        static const std::regex regex("^([+-]?[0-9]*)(n(\\^([0-9]+))?)?");

        while (!str.empty())
        {
            std::cmatch matches;
            if (!std::regex_search(str.data(), str.data() + str.length(), matches, regex) || matches.length() == 0 || matches.size() != 5)
                throw std::invalid_argument("Not a valid polynomial");

            int coefficient;
            if (matches[1] == "")
                coefficient = 1;
            else if (matches[1] == "+")
                coefficient = 1;
            else if (matches[1] == "-")
                coefficient = -1;
            else
                coefficient = std::stoi(matches[1]);
            
            int exponent;
            if (matches[2].matched)
                if (matches[3].matched)
                    exponent = std::stoi(matches[4]);
                else
                    exponent = 1;
            else
                exponent = 0;

            if (coefficients.size() < exponent + 1)
                coefficients.resize(exponent + 1, 0);
            coefficients[exponent] += coefficient;

            str = str.substr(matches.length());
        }
    }

    Polynomial::Polynomial(const Polynomial& other) : 
        coefficients(other.coefficients)
    {
    }

    Polynomial::Polynomial(Polynomial&& other) noexcept : 
        coefficients(std::move(other.coefficients))
    {
    }

    Polynomial& Polynomial::operator=(const Polynomial& other)
    {
        coefficients = other.coefficients;
        return *this;
    }

    Polynomial& Polynomial::operator=(Polynomial&& other) noexcept
    {
        coefficients = std::move(other.coefficients);
        return *this;
    }

    int Polynomial::Evaluate(int n) const
    {
        return std::ranges::fold_right(coefficients, 0, [n](int coeff, int result) { return result * n + coeff; });     
    }

    std::string Polynomial::AsString() const
    {
        if (std::ranges::all_of(coefficients, [](int c) { return c == 0; }))
            return "0";

        std::ostringstream result;

        bool firstTerm = true;

        for (int exponent = coefficients.size() - 1; exponent >= 0; --exponent)
        {
            int coefficient = coefficients[exponent];
            if (coefficient == 0)
                continue;

            if (firstTerm)
            {
                if (coefficient < 0)
                    result << '-';
            }
            else
            {
                result << (coefficient < 0 ? '-' : '+');
            }

            int absCoeff = std::abs(coefficient);
            if (exponent == 0)
            {
                result << absCoeff;
            }
            else
            {
                if (absCoeff != 1)
                    result << absCoeff;

                if (exponent >= 2)
                    result << "n^" << exponent;
                else
                    result << "n";
            }

            firstTerm = false;
        }

        return result.str();
    }
}