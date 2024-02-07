#pragma once

#include "bak/character.hpp"
#include "bak/keyContainer.hpp"
#include "bak/money.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <vector>

namespace BAK {

class Party
{
public:
    std::size_t GetNumCharacters() const
    {
        return mActiveCharacters.size();
    }

    const Character& GetCharacter(CharIndex i) const
    {
        //ASSERT(mCharacters.size() == sMaxCharacters);
        ASSERT(i.mValue < mCharacters.size());
        return mCharacters[i.mValue];
    }

    Character& GetCharacter(CharIndex i)
    {
        //ASSERT(mCharacters.size() == sMaxCharacters);
        ASSERT(i.mValue < mCharacters.size());
        return mCharacters[i.mValue];
    }

    std::optional<ActiveCharIndex> FindActiveCharacter(CharIndex index) const
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
        ASSERT(mActiveCharacters.size() <= sMaxActiveCharacters);
        return GetCharacter(mActiveCharacters[i.mValue]);
    }

    Character& GetCharacter(ActiveCharIndex i)
    {
        ASSERT(mActiveCharacters.size() <= sMaxActiveCharacters);
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

        return mKeys.GetInventory().HaveItem(item);
    }

    void GainMoney(Royals royals)
    {
        mGold.mValue += royals.mValue;
    }

    void LoseMoney(Royals royals)
    {
        if (mGold.mValue < royals.mValue)
        {
            mGold = Royals{0};
        }
        else
        {
            mGold.mValue -= royals.mValue;
        }
    }

    void RemoveItem(unsigned itemIndex, unsigned quantity)
    {
        if (ItemIndex{itemIndex} == sSovereigns)
        {
            LoseMoney(GetRoyals(Sovereigns{quantity}));
        }
        else if (ItemIndex{itemIndex} == sRoyals)
        {
            LoseMoney(Royals{quantity});
        }
        else
        {
            auto item = InventoryItemFactory::MakeItem(
                ItemIndex{itemIndex},
                static_cast<std::uint8_t>(quantity));
            if (item.IsKey())
                mKeys.RemoveItem(item);
            else
                for (const auto& character : mActiveCharacters)
                    if (GetCharacter(character).RemoveItem(item)) return;
        }
    }

    void AddItem(const InventoryItem& item)
    {
        if (item.IsMoney())
            GainItem(0, item.GetItemIndex().mValue, item.GetQuantity());
        else if (item.IsKey())
            mKeys.GiveItem(item);
    }

    void GainItem(unsigned character, unsigned itemIndex, unsigned quantity)
    {
        if (ItemIndex{itemIndex} == sSovereigns)
        {
            mGold.mValue += GetRoyals(Sovereigns{quantity}).mValue;
        }
        else if (ItemIndex{itemIndex} == sRoyals)
        {
            mGold.mValue += quantity;
        }
        else
        {
            auto baseItem = InventoryItemFactory::MakeItem(
                ItemIndex{itemIndex},
                static_cast<std::uint8_t>(quantity));

            const auto stackSize = baseItem.GetObject().mStackSize;

            // Split into stacks if necessary
            std::vector<InventoryItem> items{};
            if (baseItem.IsStackable()
                && baseItem.GetQuantity() > stackSize)
            {
                const auto nStacks = baseItem.GetQuantity() / stackSize;
                for (unsigned i = 0; i < nStacks; i++)
                {
                    items.emplace_back(
                        InventoryItemFactory::MakeItem(
                            ItemIndex{itemIndex},
                            stackSize));
                }
                const auto remainder = baseItem.GetQuantity() % stackSize;
                if (remainder != 0)
                    items.emplace_back(
                        InventoryItemFactory::MakeItem(
                            ItemIndex{itemIndex},
                            remainder));
            }
            else
            {
                items.emplace_back(baseItem);
            }

            for (const auto& item : items)
            {
                if (item.IsKey())
                {
                    mKeys.GiveItem(item);
                }
                else
                {
                    for (const auto& character : mActiveCharacters)
                        if (GetCharacter(character).GiveItem(item))
                            break;
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

    template <typename F>
    void ForEachActiveCharacter(F&& f) const
    {
        for (const auto character : mActiveCharacters)
        {
            if (std::forward<F>(f)(GetCharacter(character)))
            {
                break;
            }
        }
    }

    template <typename F>
    void ForEachActiveCharacter(F&& f)
    {
        for (const auto character : mActiveCharacters)
        {
            if (std::forward<F>(f)(GetCharacter(character)))
            {
                break;
            }
        }
    }

    std::pair<CharIndex, unsigned> GetSkill(BAK::SkillType skill, bool best)
    {
        std::optional<unsigned> skillValue{};
        auto character = CharIndex{0};
        for (unsigned i = 0; i < mActiveCharacters.size(); i++)
        {
            const auto charSkill = GetCharacter(ActiveCharIndex{i}).GetSkill(skill);
            Logging::LogDebug(__FUNCTION__) << "Char: " << GetCharacter(ActiveCharIndex{i}).mName << " skill: " << charSkill << "\n";
            if (!skillValue
                || best
                    ? charSkill > skillValue
                    : charSkill < skillValue)
            {
                skillValue = charSkill;
                character = mActiveCharacters[i];
            }
        }
        ASSERT(skillValue);
        return std::make_pair(character, *skillValue);
    }

    void ImproveSkillForAll(SkillType skill, SkillChange skillChangeType, int multiplier)
    {
        for (const auto c : mActiveCharacters)
        {
            GetCharacter(c).ImproveSkill(
                skill,
                skillChangeType,
                multiplier);
        }
    }

    Royals mGold;
    KeyContainer mKeys;

    std::vector<Character> mCharacters;
    std::vector<CharIndex> mActiveCharacters;
};

std::ostream& operator<<(std::ostream&, const Party&);

}
