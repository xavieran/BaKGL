#include "com/string.hpp"

#include <cctype>
#include <iterator>

std::string ToUpper(std::string_view str)
{
    std::string uppered{};

    std::transform(
        str.begin(),
        str.end(),
        std::back_inserter(uppered),
        [](auto c){ return std::toupper(c); });

    return uppered;
}
