#pragma once

#include <array>
#include <iomanip>
#include <ostream>

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

}
