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

    unsigned NextActiveCharacter(unsigned currentCharacter)
    {
        const auto current = std::find(
            mActiveCharacters.begin(), mActiveCharacters.end(),
            currentCharacter);
        assert(current != mActiveCharacters.end());
        unsigned i = std::distance(mActiveCharacters.begin(), current);
        if (++i == mActiveCharacters.size())
            i = 0;
        return mActiveCharacters[i];
    }
};

std::ostream& operator<<(std::ostream&, const Party&);

}
