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

    const Character& GetActiveCharacter(unsigned i) const
    {
        ASSERT(i < mActiveCharacters.size());
        const auto activeIndex = mActiveCharacters[i];
        ASSERT(activeIndex < mCharacters.size());
        return mCharacters[activeIndex];
    }

    Character& GetActiveCharacter(unsigned i)
    {
        ASSERT(i < mActiveCharacters.size());
        const auto activeIndex = mActiveCharacters[i];
        ASSERT(activeIndex < mCharacters.size());
        return mCharacters[activeIndex];
    }

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

    void GainItem(unsigned character, unsigned itemIndex, unsigned quantity)
    {
        if (itemIndex == 0x35)
        {
            mGold.mValue += GetRoyals(Sovereigns{quantity}).mValue;
        }
        else if (itemIndex == 0x36)
        {
            mGold.mValue += quantity;
        }
        else
        {
            bool given = false;
            unsigned character = 0;
            auto item = InventoryItemFactory::MakeItem(
                ItemIndex{itemIndex},
                static_cast<std::uint8_t>(quantity));
            for (const auto& character : mActiveCharacters)
            {
                if (GetActiveCharacter(character).GiveItem(std::move(item)))
                    return;
            }
        }
    }

    unsigned NextActiveCharacter(unsigned currentCharacter)
    {
        unsigned i = currentCharacter;
        if (++i == mActiveCharacters.size())
            i = 0;
        return i;
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
