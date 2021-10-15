#pragma once

#include "bak/character.hpp"
#include "bak/money.hpp"

#include "com/assert.hpp"

#include <vector>

namespace BAK {

class Party
{
public:
    using CharacterIndex = std::uint8_t;

    Royals mGold;
    Inventory mKeys;

    std::vector<Character> mCharacters;
    std::vector<CharacterIndex> mActiveCharacters;

    Royals GetGold() const
    {
        return mGold;
    }

    void RemoveItem(unsigned item, unsigned quantity)
    {
        if (item == 0x35)
        {
            mGold.mValue -= GetRoyals(Sovereigns{quantity}).mValue;
        }
        else if (item == 0x36)
        {
            mGold.mValue -= quantity;
        }
        else
        {
            // for (auto& inventory : mActiveCharacters inventory...
            // if (try remove item) break
        }
    }

    void GainItem(unsigned character, unsigned item, unsigned quantity)
    {
        if (item == 0x35)
        {
            mGold.mValue += GetRoyals(Sovereigns{quantity}).mValue;
        }
        else if (item == 0x36)
        {
            mGold.mValue += quantity;
        }
        else
        {
            // mCharacters[character].GiveITems()...
        }
    }

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

    std::pair<CharacterIndex, unsigned> GetSkill(BAK::SkillType skill, bool best)
    {
        std::optional<unsigned> skillValue{};
        CharacterIndex character = 0;
        for (unsigned i = 0; i < mActiveCharacters.size(); i++)
        {
            const auto charSkill = mCharacters[i].mSkills.GetSkill(skill).mCurrent;
            if (!skillValue
                || best
                    ? charSkill > skillValue
                    : charSkill < skillValue)
            {
                skillValue = charSkill;
                character = i;
            }
        }
        ASSERT(skillValue);
        return std::make_pair(character, *skillValue);
    }
};

std::ostream& operator<<(std::ostream&, const Party&);

}
