#pragma once

#include <array>
#include <iomanip>
#include <ostream>
#include <optional>
#include <vector>

namespace std {

template <typename T, std::size_t N>
inline ostream& operator<<(ostream& os, const array<T, N>& a)
{
    string sep = "";
    for (unsigned i = 0; i < N; i++)
    {
        os << sep << setw(2) << setfill('0') << +a[i];
        sep = " ";
    }
    return os;
}

template <typename T>
inline ostream& operator<<(ostream& os, const vector<T>& items)
{
    string sep = "";
    for (const auto& item : items)
    {
        os << sep << item;
        sep = ",";
    }
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& o)
{
    if (o)
        os << "[[" << *o << "]]";
    else
        os << "[[null]]";
    return os;
}

}
