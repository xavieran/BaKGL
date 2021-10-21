#pragma once

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "bak/IContainer.hpp"

#include "bak/condition.hpp"
#include "bak/skills.hpp"
#include "bak/inventory.hpp"

#include <cstdint>
#include <ios>
#include <iterator>
#include <ostream>
#include <string>

namespace BAK {

class Character : public IContainer
{
public:
    Character(
        unsigned index,
        const std::string& name,
        const Skills& skills,
        const std::array<std::uint8_t, 6>& spells,
        const std::array<std::uint8_t, 2>& unknown,
        const std::array<std::uint8_t, 7>& unknown2,
        const Conditions& conditions,
        const Inventory& inventory
        )
    :
        mCharacterIndex{index},
        mName{name},
        mSkills{skills},
        mSpells{spells},
        mUnknown{unknown},
        mUnknown2{unknown2},
        mConditions{conditions},
        mInventory{inventory}
    {}

    /* IContainer */
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool GiveItem(InventoryItem item)
    {
        // FIXME: Check character class...
        if (CanReplaceEquippableItem(item.GetObject().mType)
            && mInventory.FindEquipped(item.GetObject().mType)
                == mInventory.GetItems().end())
        {
            item.SetEquipped(true);
        }
        else
        {
            item.SetEquipped(false);
        }

        if (mInventory.CanAdd(item))
        {
            mInventory.AddItem(item);
            return true;
        }

        return false;
    }

    bool RemoveItem(const InventoryItem& item)
    {
        if (mInventory.HaveItem(item))
        {
            mInventory.RemoveItem(item);
            return true;
        }

        return false;
    }

    bool IsSpellcaster() const { return mSkills.GetSkill(BAK::SkillType::Casting).mCurrent != 0; }
    bool IsSwordsman() const { return !IsSpellcaster(); }
    ItemType GetWeaponType() const
    {
        if (IsSpellcaster())
            return ItemType::Staff;
        else
            return ItemType::Sword;
    }

    bool CanReplaceEquippableItem(ItemType type) const
    {
        if (IsSpellcaster() && type == ItemType::Staff)
            return true;
        else if (IsSwordsman() && (type == ItemType::Sword
                || type == ItemType::Crossbow))
            return true;
        else if (type == ItemType::Armor)
            return true;
        return false;
    }

    void ApplyItemToSlot(InventoryIndex index, BAK::ItemType slot)
    {
        auto& item = mInventory.GetAtIndex(index);
        Logging::LogDebug("CharacterB4Move") << __FUNCTION__ << " " << item << " " << item.IsEquipped() << " " << BAK::ToString(slot) << "\n";
        auto equipped = mInventory.FindEquipped(slot);
        // We are trying to move this item onto itself
        if (std::distance(mInventory.GetItems().begin(), equipped) == index.mValue)
        {
            return;
        }

        if (equipped != mInventory.GetItems().end())
            Logging::LogDebug("CharacterB4Eqip") << __FUNCTION__ << " " << *equipped << " " << equipped->IsEquipped() << " " << BAK::ToString(slot) << "\n";
        if (item.GetObject().mType == slot)
        {
            item.SetEquipped(true);
            if (equipped != mInventory.GetItems().end())
                equipped->SetEquipped(false);
        }
        else
        {
            // Try use item at index on slot item
        }

        Logging::LogDebug("CharacterAFMove") << __FUNCTION__ << " " << item << " " << BAK::ToString(slot) << "\n";
        if (equipped != mInventory.GetItems().end())
            Logging::LogDebug("CharacterAFEquip") << __FUNCTION__ << " " << *equipped << " " << equipped->IsEquipped() << " " << BAK::ToString(slot) << "\n";
    }

    void CheckPostConditions()
    {
        if (IsSpellcaster())
        {
            ASSERT(GetInventory().FindEquipped(ItemType::Sword) 
                == GetInventory().GetItems().end());
            ASSERT(GetInventory().FindEquipped(ItemType::Crossbow) 
                == GetInventory().GetItems().end());

            unsigned staffCount = 0;
            for (const auto& item : GetInventory().GetItems())
            {
                if (item.GetObject().mType == ItemType::Staff
                    && item.IsEquipped())
                {
                    staffCount++;
                }
            }
            ASSERT(staffCount == 1);
        }
        else
        {
            ASSERT(GetInventory().FindEquipped(ItemType::Staff) 
                == GetInventory().GetItems().end());

            unsigned swordCount = 0;
            for (const auto& item : GetInventory().GetItems())
            {
                if (item.GetObject().mType == ItemType::Sword
                    && item.IsEquipped())
                {
                    swordCount++;
                }
            }
            ASSERT(swordCount == 1);

            unsigned crossbowCount = 0;
            for (const auto& item : GetInventory().GetItems())
            {
                if (item.GetObject().mType == ItemType::Crossbow
                    && item.IsEquipped())
                {
                    crossbowCount++;
                }
            }

            ASSERT(crossbowCount <= 1);
        }
    }

    unsigned mCharacterIndex;
    std::string mName;
    Skills mSkills;
    std::array<std::uint8_t, 6> mSpells;
    std::array<std::uint8_t, 2> mUnknown;
    std::array<std::uint8_t, 7> mUnknown2;
    Conditions mConditions;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream&, const Character&);

}
