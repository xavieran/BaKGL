#pragma once

#include "bak/objectInfo.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/types.hpp"

#include "com/bits.hpp"

#include <iostream>

namespace BAK {

enum class ItemStatus : std::uint8_t
{
    Broken     = 4,
    Repairable = 5,
    Equipped   = 6,
    Poisoned   = 7
};

enum class ItemFlags : std::uint16_t
{
    Stackable      = 11, // 0x0800
    ConditionBased = 12, // 0x1000
    ChargeBased    = 13, // 0x2000
    QuantityBased  = 15, // 0x8000
};

bool CheckItemStatus(std::uint8_t status, ItemStatus flag);
std::uint8_t SetItemStatus(std::uint8_t status, ItemStatus flag, bool state);

class InventoryItem
{
public:
    InventoryItem(
        GameObject const* object,
        ItemIndex itemIndex,
        unsigned condition,
        std::uint8_t status,
        std::uint8_t modifiers);

    const GameObject& GetObject() const { ASSERT(mObject); return *mObject; }

    auto GetItemIndex() const { return mItemIndex; }
    auto GetQuantity() const { return mCondition; }
    auto GetSpell() const { return SpellIndex{mCondition}; }
    auto GetCondition() const { return mCondition; }
    auto GetStatus() const { return mStatus; }
    auto GetModifierMask() const { return mModifiers; }

    bool IsEquipped() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Equipped);
    }

    bool IsBroken() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Broken);
    }

    bool IsRepairable() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Repairable) && !IsBroken();
    }

	bool IsRepairableByShop() const
    {
        return GetCondition() != 100;
    }

    bool IsPoisoned() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Poisoned);
    }

    bool IsMoney() const
    {
        return mItemIndex == ItemIndex{0x35}
            || mItemIndex == ItemIndex{0x36};
    }

    bool IsKey() const
    {
        return IsItemType(ItemType::Key) || mItemIndex == BAK::sPicklock;
    }

    void SetEquipped(bool state)
    {
        mStatus = SetItemStatus(mStatus, ItemStatus::Equipped, state);
    }

    void SetRepairable(bool state)
    {
        mStatus = SetItemStatus(mStatus, ItemStatus::Repairable, state);
    }

    void SetBroken(bool state)
    {
        mStatus = SetItemStatus(mStatus, ItemStatus::Broken, state);
    }

    void SetCondition(unsigned condition)
    {
        mCondition = condition;
    }

    void SetQuantity(unsigned quantity)
    {
        ASSERT(!IsStackable() || (IsStackable() && quantity <= GetObject().mStackSize));
        mCondition = quantity;
    }

    bool HasFlag(ItemFlags flag) const
    {
        return CheckBitSet(GetObject().mFlags, flag);
    }

    bool IsConditionBased() const
    {
        return HasFlag(ItemFlags::ConditionBased);
    }

    bool IsChargeBased() const
    {
        return HasFlag(ItemFlags::ChargeBased);
    }

    bool IsStackable() const
    {
        return HasFlag(ItemFlags::Stackable);
    }

    bool IsQuantityBased() const
    {
        return HasFlag(ItemFlags::QuantityBased);
    }

    bool IsSkillModifier() const
    {
        return GetObject().mModifierMask != 0;
    }

    bool IsItemType(BAK::ItemType type) const
    {
        return GetObject().mType == type;
    }

    bool IsItemModifier() const
    {
        return IsItemType(ItemType::WeaponOil)
            || IsItemType(ItemType::ArmorOil)
            || IsItemType(BAK::ItemType::SpecialOil);
    }

    bool IsModifiableBy(ItemType itemType)
    {
        return (itemType == ItemType::WeaponOil && IsItemType(ItemType::Sword))
            || (itemType == ItemType::ArmorOil && IsItemType(ItemType::Armor))
            || (itemType == ItemType::SpecialOil 
                    && (IsItemType(ItemType::Armor)
                        || IsItemType(ItemType::Sword)));
    }

    bool IsRepairItem() const
    {
        return IsItemType(BAK::ItemType::Tool);
    }

    bool DisplayCondition() const
    {
        return IsConditionBased();
    }

    bool DisplayNumber() const
    {
        return IsConditionBased() 
            || IsStackable()
            || IsChargeBased()
            || IsQuantityBased()
            || IsKey();
    }

    bool HasModifier(Modifier mod) const
    {
        return CheckBitSet(mModifiers, mod);
    }

    void ClearTemporaryModifiers()
    {
        mStatus = mStatus & 0b0111'1111;
        mModifiers = mModifiers & 0b1110'0000;
    }

    void SetStatusAndModifierFromMask(std::uint16_t mask)
    {
        mStatus |= (mask & 0xff);
        mModifiers |= ((mask >> 8) & 0xff);
    }

    void SetModifier(Modifier mod)
    {
        mModifiers = SetBit(mModifiers, static_cast<std::uint8_t>(mod), true);
    }

    void UnsetModifier(Modifier mod)
    {
        mModifiers = SetBit(mModifiers, static_cast<std::uint8_t>(mod), false);
    }

    std::vector<Modifier> GetModifiers() const
    {
        auto mods = std::vector<Modifier>{};
        for (unsigned i = 0; i < 8; i++)
            if (CheckBitSet(mModifiers, i))
                mods.emplace_back(static_cast<Modifier>(i));
        return mods;
    }

    std::pair<unsigned, unsigned> GetItemUseSound()
    {
        return std::make_pair(GetObject().mUseSound, GetObject().mSoundPlayTimes);
    }


private:
    GameObject const* mObject;
    ItemIndex mItemIndex;
    unsigned mCondition;
    std::uint8_t mStatus;
    std::uint8_t mModifiers;
};

std::ostream& operator<<(std::ostream&, const InventoryItem&);

class InventoryItemFactory
{
public:
    static InventoryItem MakeItem(
        ItemIndex itemIndex,
        std::uint8_t quantity,
        std::uint8_t status,
        std::uint8_t modifiers)
    {
        const auto& objects = GetObjectIndex();

        return InventoryItem{
            &objects.GetObject(itemIndex),
            itemIndex,
            quantity,
            status,
            modifiers};
    }

    static InventoryItem MakeItem(
        ItemIndex itemIndex,
        std::uint8_t quantity)
    {
        return MakeItem(
            itemIndex,
            quantity,
            0,
            0);
    }
};

}
