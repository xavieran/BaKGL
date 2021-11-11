#pragma once

#include "bak/character.hpp"
#include "bak/keyContainer.hpp"
#include "bak/money.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/strongType.hpp"

#include <vector>

namespace BAK {

class Party
{
public:
    const Character& GetCharacter(CharIndex i) const
    {
        assert(mCharacters.size() == sMaxCharacters);
        return mCharacters[i.mValue];
    }

    Character& GetCharacter(CharIndex i)
    {
        assert(mCharacters.size() == sMaxCharacters);
        return mCharacters[i.mValue];
    }

    std::optional<ActiveCharIndex> FindActiveCharacter(CharIndex index)
    {
        const auto it = std::find(mActiveCharacters.begin(), mActiveCharacters.end(), index);
        if (it != mActiveCharacters.end())
            return ActiveCharIndex{
                static_cast<unsigned>(
                    std::distance(mActiveCharacters.begin(), it))};
        else
            return std::optional<ActiveCharIndex>{};
    }

    const Character& GetCharacter(ActiveCharIndex i) const
    {
        assert(mActiveCharacters.size() <= sMaxActiveCharacters);
        return GetCharacter(mActiveCharacters[i.mValue]);
    }

    Character& GetCharacter(ActiveCharIndex i)
    {
        assert(mActiveCharacters.size() <= sMaxActiveCharacters);
        return GetCharacter(mActiveCharacters[i.mValue]);
    }

    const auto& GetKeys() const
    {
        return mKeys;
    }

    auto& GetKeys()
    {
        return mKeys;
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

    void GainMoney(Royals royals)
    {
        mGold.mValue += royals.mValue;
    }

    void LoseMoney(Royals royals)
    {
        ASSERT(royals.mValue < mGold.mValue);
        mGold.mValue -= royals.mValue;
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

    void AddItem(const InventoryItem& item)
    {
        if (item.IsMoney())
            GainItem(0, item.mItemIndex.mValue, item.GetQuantity());
        else if (item.IsKey())
            mKeys.GiveItem(item);
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
            if (item.IsKey())
            {
                mKeys.GiveItem(item);
            }
            else
            {
                for (const auto& character : mActiveCharacters)
                {
                    if (GetCharacter(character).GiveItem(item))
                        return;
                }
            }
        }
    }

    ActiveCharIndex NextActiveCharacter(ActiveCharIndex currentCharacter) const
    {
        unsigned i = currentCharacter.mValue;
        if (++i == mActiveCharacters.size())
            i = 0;
        return ActiveCharIndex{i};
    }

    std::pair<CharIndex, unsigned> GetSkill(BAK::SkillType skill, bool best)
    {
        std::optional<unsigned> skillValue{};
        std::uint8_t character{0};
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
        return std::make_pair(CharIndex{character}, *skillValue);
    }

    Royals mGold;
    KeyContainer mKeys;

    std::vector<Character> mCharacters;
    std::vector<CharIndex> mActiveCharacters;
};

std::ostream& operator<<(std::ostream&, const Party&);

}
