#pragma once

#include <string>
#include <string_view>
#include <vector>

std::string ToUpper(std::string_view str);

std::vector<std::string> SplitString(
    std::string delim,
    std::string input);
