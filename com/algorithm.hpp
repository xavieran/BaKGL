#pragma once

#include <algorithm>

template <typename T, typename It>
It find_nth(It begin, It end, const T& needle, unsigned n)
{
    assert(n > 0);
    do
    {
        begin = std::find(begin, end, needle);
        if (begin != end) begin++;
        else break;
    } while ((--n) > 0);

    return begin;
}
