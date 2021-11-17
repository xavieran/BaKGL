#pragma once

#include "bak/objectInfo.hpp"
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

    auto GetQuantity() const { return mCondition; }
    auto GetCondition() const { return mCondition; }

    bool IsEquipped() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Equipped);
    }

    bool IsRepairable() const
    {
        return CheckItemStatus(mStatus, ItemStatus::Repairable);
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
        return GetObject().mType == ItemType::Key 
            || mItemIndex.mValue == 'P';
    }

    void SetEquipped(bool state)
    {
        mStatus = SetItemStatus(mStatus, ItemStatus::Equipped, state);
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

    bool HasModifier(Modifier mod)
    {
        return CheckBitSet(mModifiers, mod);
    }

    std::vector<Modifier> GetModifiers() const
    {
        auto mods = std::vector<Modifier>{};
        for (unsigned i = 0; i < 8; i++)
            if (CheckBitSet(mModifiers, i))
                mods.emplace_back(static_cast<Modifier>(i));
        return mods;
    }

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
