#pragma once

#include "bak/character.hpp"
#include "bak/keyContainer.hpp"
#include "bak/money.hpp"

#include "com/assert.hpp"

#include <vector>

namespace BAK {

class Party
{
public:
    using CharacterIndex = std::uint8_t;

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

    const Character& GetCharacter(unsigned i) const
    {
        ASSERT(i < mCharacters.size());
        return mCharacters[i];
    }

    const auto& GetKeys() const
    {
        return mKeys;
    }

    auto& GetKeys()
    {
        return mKeys;
    }
    Character& GetCharacter(unsigned i)
    {
        ASSERT(i < mCharacters.size());
        return mCharacters[i];
    }

    Royals GetGold() const
    {
        return mGold;
    }

    bool HaveItem(ItemIndex itemIndex) const
    {
        const auto item = InventoryItemFactory::MakeItem(
            itemIndex,
            1);

        for (const auto& character : mActiveCharacters)
            if (GetCharacter(character).GetInventory().HaveItem(item))
                return true;

        return false;
    }

    void RemoveItem(unsigned itemIndex, unsigned quantity)
    {
        if (itemIndex == 0x35)
        {
            mGold.mValue -= GetRoyals(Sovereigns{quantity}).mValue;
        }
        else if (itemIndex == 0x36)
        {
            mGold.mValue -= quantity;
        }
        else
        {
            auto item = InventoryItemFactory::MakeItem(
                ItemIndex{itemIndex},
                static_cast<std::uint8_t>(quantity));
            for (const auto& character : mActiveCharacters)
            {
                if (GetCharacter(character).RemoveItem(item))
                    return;
            }
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
            auto item = InventoryItemFactory::MakeItem(
                ItemIndex{itemIndex},
                static_cast<std::uint8_t>(quantity));
            for (const auto& character : mActiveCharacters)
            {
                if (GetCharacter(character).GiveItem(item))
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

    Royals mGold;
    KeyContainer mKeys;

    std::vector<Character> mCharacters;
    std::vector<CharacterIndex> mActiveCharacters;


};

std::ostream& operator<<(std::ostream&, const Party&);

}
