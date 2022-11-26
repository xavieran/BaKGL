#pragma once

#include "com/logger.hpp"

#include "bak/IContainer.hpp"

#include "bak/condition.hpp"
#include "bak/skills.hpp"
#include "bak/types.hpp"
#include "bak/inventory.hpp"

#include <cstdint>
#include <iterator>
#include <ostream>
#include <string>

namespace BAK {

class Character final : public IContainer
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
        Inventory&& inventory)
    :
        mCharacterIndex{index},
        mName{name},
        mSkills{skills},
        mSpells{spells},
        mUnknown{unknown},
        mUnknown2{unknown2},
        mConditions{conditions},
        mInventory{std::move(inventory)}
    {}

    /* IContainer */

    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool CanSwapItem(const InventoryItem& ref) const
    {
        return (ref.GetObject().mType == BAK::ItemType::Sword && IsSwordsman())
            || (ref.GetObject().mType == BAK::ItemType::Staff && IsSpellcaster());
    }

    bool CanAddItem(const InventoryItem& ref) const override
    {
        auto item = ref;
        // Day's rations are equivalent to 1 of normal rations
        if (ref.mItemIndex == ItemIndex{134})
        {
            item = InventoryItemFactory::MakeItem(ItemIndex{72}, 1);
        }

        if (mInventory.CanAddCharacter(item) > 0)
            return true;
        else if (item.GetObject().mType == ItemType::Staff
            && HasEmptyStaffSlot())
            return true;
        else if (item.GetObject().mType == ItemType::Sword
            && HasEmptySwordSlot())
            return true;
        else if (item.GetObject().mType == ItemType::Crossbow
            && HasEmptyCrossbowSlot())
            return true;
        else if (item.GetObject().mType == ItemType::Armor
            && HasEmptyArmorSlot())
            return true;
        else
            return false;
    }

    bool GiveItem(const InventoryItem& ref) override
    {
        auto item = ref;

        bool equipped = false;
        if (CanReplaceEquippableItem(item.GetObject().mType)
            && mInventory.FindEquipped(item.GetObject().mType)
                == mInventory.GetItems().end())
        {
            item.SetEquipped(true);
            equipped = true;
        }
        else
        {
            item.SetEquipped(false);
        }

        // Day's rations are equivalent to 1 of normal rations
        if (ref.mItemIndex == ItemIndex{134})
        {
            item = InventoryItemFactory::MakeItem(ItemIndex{72}, 1);
        }
        // Quegian brandy
        else if (ref.mItemIndex == ItemIndex{135}
            || ref.mItemIndex == ItemIndex{136}
            || ref.mItemIndex == ItemIndex{137})
        {
            mConditions.IncreaseCondition(
                static_cast<Condition>(ref.GetObject().mEffectMask),
                ref.GetObject().mEffect);
            return true;
        }

        if (mInventory.CanAddCharacter(item)
            || equipped)
        {
            mInventory.AddItem(item);
            return true;
        }

        return false;
    }

    bool RemoveItem(const InventoryItem& item) override
    {
        if (mInventory.HaveItem(item))
        {
            mInventory.RemoveItem(item);
            return true;
        }

        return false;
    }

    ContainerType GetContainerType() const override
    {
        return ContainerType::Inv;
    }

    ShopStats& GetShop() override { ASSERT(false); return *reinterpret_cast<ShopStats*>(this);}
    const ShopStats& GetShop() const override { ASSERT(false); return *reinterpret_cast<const ShopStats*>(this);}
    LockStats& GetLock() override { ASSERT(false); return *reinterpret_cast<LockStats*>(this); }
    /* Character Getters */

    CharIndex GetIndex() const { return mCharacterIndex; }

    bool IsSpellcaster() const { return mSkills.GetSkill(BAK::SkillType::Casting).mMax != 0; }
    bool IsSwordsman() const { return !IsSpellcaster(); }

    bool HasEmptyStaffSlot() const
    {
        return IsSpellcaster() 
            && mInventory.FindEquipped(ItemType::Staff) 
                == mInventory.GetItems().end();
    }

    bool HasEmptySwordSlot() const
    {
        return IsSwordsman() 
            && mInventory.FindEquipped(ItemType::Sword) 
                == mInventory.GetItems().end();
    }

    bool HasEmptyCrossbowSlot() const
    {
        return IsSwordsman() 
            && mInventory.FindEquipped(ItemType::Crossbow) 
                == mInventory.GetItems().end();
    }

    bool HasEmptyArmorSlot() const
    {
        return mInventory.FindEquipped(ItemType::Armor) 
            == mInventory.GetItems().end();
    }

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

    void ApplyItemToSlot(InventoryIndex index, ItemType slot)
    {
        auto& item = mInventory.GetAtIndex(index);
        auto equipped = mInventory.FindEquipped(slot);
        // We are trying to move this item onto itself
        if (std::distance(mInventory.GetItems().begin(), equipped) == index.mValue)
        {
            return;
        }

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

    const std::string& GetName() const
    {
        return mName;
    }

    const Skills& GetSkills() const
    {
        return mSkills;
    }

    void ImproveSkill(SkillType skill, SkillChange skillChangeType, unsigned multiplier)
    {
        mSkills.ImproveSkill(skill, skillChangeType, multiplier);
        UpdateSkills();
    }

    unsigned GetSkill(SkillType skill) const
    {
        mSkills.GetSkill(skill).mModifier = mInventory.CalculateModifiers(skill);
        return CalculateEffectiveSkillValue(
            skill,
            mSkills,
            mConditions,
            SkillRead::Current);
    }

    unsigned GetMaxSkill(SkillType skill) const
    {
        mSkills.GetSkill(skill).mModifier = mInventory.CalculateModifiers(skill);
        return CalculateEffectiveSkillValue(
            skill,
            mSkills,
            mConditions,
            SkillRead::MaxSkill);
    }

    const Conditions& GetConditions() const { return mConditions; }

    void UpdateSkills()
    {
        for (unsigned i = 0; i < BAK::Skills::sSkills; i++)
            GetSkill(static_cast<SkillType>(i));
    }

    CharIndex mCharacterIndex;
    std::string mName;
    mutable Skills mSkills;
    std::array<std::uint8_t, 6> mSpells;
    std::array<std::uint8_t, 2> mUnknown;
    std::array<std::uint8_t, 7> mUnknown2;
    Conditions mConditions;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream&, const Character&);

}
