#pragma once

#include "bak/character.hpp"

#include <vector>

namespace BAK {

class Party
{
public:
    std::vector<std::reference_wrapper<Character>> mActiveCharacters;
    std::vector<Character> mCharacters;

    // Party gold
    // Keys
};

}
