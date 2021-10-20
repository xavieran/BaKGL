#pragma once

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
    bool GiveItem(const InventoryItem& item)
    {
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
