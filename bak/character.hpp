#pragma once

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "bak/condition.hpp"
#include "bak/skills.hpp"
#include "bak/inventory.hpp"

#include <ostream>
#include <ios>
#include <string>

namespace BAK {

class Character
{
public:
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

    auto& GetInventory() { return mInventory; }
    const auto& GetInventory() const { return mInventory; }
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
