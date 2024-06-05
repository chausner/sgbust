#include "core/Polynom.h"

#include <cmath>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sgbust
{
    Polynom::Polynom(std::initializer_list<int> coefficients) : coefficients(coefficients)
    {
    }

    Polynom::Polynom(std::string_view str)
    {
        static const std::regex regex("^([+-]?[0-9]*)(n(\\^([0-9]+))?)?");

        while (!str.empty())
        {
            std::cmatch matches;
            if (!std::regex_search(str.data(), str.data() + str.length(), matches, regex) || matches.length() == 0 || matches.size() != 5)
                throw std::invalid_argument("Not a valid polynom");

            int coefficient;
            if (matches[1].length() == 0)
                coefficient = 1;
            else if (matches[1].str() == "+" || matches[1].str() == "-")
                coefficient = std::stoi(matches[1].str() + '1');
            else
                coefficient = std::stoi(matches[1].str());
            
            int exponent;
            if (matches[2].matched)
                if (matches[3].matched)
                    exponent = std::stoi(matches[4].str());
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

    Polynom::Polynom(const Polynom& other) : coefficients(other.coefficients)
    {
    }

    Polynom::Polynom(Polynom&& other) noexcept : coefficients(std::move(other.coefficients))
    {

    }

    Polynom& Polynom::operator=(const Polynom& other)
    {
        coefficients = other.coefficients;
        return *this;
    }

    Polynom& Polynom::operator=(Polynom&& other) noexcept
    {
        coefficients = std::move(other.coefficients);
        return *this;
    }

    int Polynom::Evaluate(int n) const
    {
        switch (coefficients.size())
        {
        case 0:
            return 0;
        case 1:
            return coefficients[0];
        case 2:
            return coefficients[0] + coefficients[1] * n;
        case 3:
            return coefficients[0] + (coefficients[1] + coefficients[2] * n) * n;
        case 4:
            return coefficients[0] + (coefficients[1] + (coefficients[2] + coefficients[3] * n) * n) * n;
        case 5:
            return coefficients[0] + (coefficients[1] + (coefficients[2] + (coefficients[3] + coefficients[4] * n) * n) * n) * n;
        default:
            int result = 0;
            for (int i = coefficients.size() - 1; i >= 0; i--)
            {            
                result = result * n + coefficients[i];
            }
            return result;
        }
    }

    std::string Polynom::AsString() const
    {
        std::ostringstream result;

        for (int i = 0; i < coefficients.size(); i++)
        {
            if (coefficients[i] == 0)
                continue;

            if (i == 0 && coefficients[i] < 0)
                result << '-';
            else if (i > 0)
                result << (coefficients[i] < 0 ? '-' : '+');

            result << std::abs(coefficients[i]);

            int exponent = coefficients.size() - i - 1;
            if (exponent >= 2)
                result << "n^" << exponent;
            else if (exponent == 1)
                result << "n";
        }

        return result.str();
    }
}