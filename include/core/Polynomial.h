#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace sgbust
{
    class Polynomial
    {
        std::vector<int> coefficients;

    public:
        Polynomial() {}
        Polynomial(std::initializer_list<int> coefficients);
        Polynomial(std::string_view str);

        Polynomial(const Polynomial& other);
        Polynomial(Polynomial&& other) noexcept;
        Polynomial& operator=(const Polynomial& other);
        Polynomial& operator=(Polynomial&& other) noexcept;

        int Evaluate(int n) const;
        std::string AsString() const;
    };
}