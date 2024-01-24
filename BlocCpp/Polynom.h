#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

class Polynom
{
    std::vector<int> coefficients;

public:
    Polynom() {}
    Polynom(std::initializer_list<int> coefficients);
    Polynom(std::string_view str);

    Polynom(const Polynom& other);
    Polynom(Polynom&& other) noexcept;
    Polynom& operator=(const Polynom& other);
    Polynom& operator=(Polynom&& other) noexcept;

    int Evaluate(int n) const;
    std::string AsString() const;
};