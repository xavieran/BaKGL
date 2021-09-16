#pragma once

#include "bak/character.hpp"

#include <vector>

namespace BAK {

class Party
{
public:
    using CharacterIndex = std::uint8_t;

    int mGold;
    Inventory mKeys;

    std::vector<Character> mCharacters;
    std::vector<CharacterIndex> mActiveCharacters;
};

std::ostream& operator<<(std::ostream&, const Party&);

}
