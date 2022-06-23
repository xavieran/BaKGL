#include "com/string.hpp"

#include <cctype>
#include <iterator>
#include <algorithm>

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

std::vector<std::string> SplitString(
    std::string delim,
    std::string input)
{
    std::vector<std::string> words{};

    size_t pos = 0;
    while ((pos = input.find(delim)) != std::string::npos) 
    {
        words.push_back(input.substr(0, pos));
        input.erase(0, pos + delim.length());
    }

    if (!input.empty())
        words.emplace_back(input);

    return words;
}
