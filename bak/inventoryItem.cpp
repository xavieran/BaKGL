#include "bak/inventoryItem.hpp"

#include "bak/itemNumbers.hpp"
#include "bak/objectInfo.hpp"

#include "com/assert.hpp"
#include "com/bits.hpp"
#include "com/ostream.hpp"

namespace BAK {

bool CheckItemStatus(std::uint8_t status, ItemStatus flag)
{
    return CheckBitSet(status, flag);
}

std::uint8_t SetItemStatus(std::uint8_t status, ItemStatus flag, bool state)
{
    return SetBit(status, flag, state);
}

InventoryItem::InventoryItem(
    GameObject const* object,
    ItemIndex itemIndex,
    unsigned condition,
    std::uint8_t status,
    std::uint8_t modifiers)
:
    mObject{object},
    mItemIndex{itemIndex},
    mCondition{condition},
    mStatus{status},
    mModifiers{modifiers}
{}

const GameObject& InventoryItem::GetObject() const { ASSERT(mObject); return *mObject; }

ItemIndex InventoryItem::GetItemIndex() const { return mItemIndex; }
unsigned InventoryItem::GetQuantity() const { return mCondition; }
SpellIndex InventoryItem::GetSpell() const { return SpellIndex{mCondition}; }
unsigned InventoryItem::GetCondition() const { return mCondition; }
std::uint8_t InventoryItem::GetStatus() const { return mStatus; }
std::uint8_t InventoryItem::GetModifierMask() const { return mModifiers; }

bool InventoryItem::IsActivated() const
{
    return CheckItemStatus(mStatus, ItemStatus::Activated);
}

bool InventoryItem::IsEquipped() const
{
    return CheckItemStatus(mStatus, ItemStatus::Equipped);
}

bool InventoryItem::IsBroken() const
{
    return CheckItemStatus(mStatus, ItemStatus::Broken);
}

bool InventoryItem::IsRepairable() const
{
    return CheckItemStatus(mStatus, ItemStatus::Repairable) && !IsBroken();
}

bool InventoryItem::IsRepairableByShop() const
{
    return GetCondition() != 100;
}

bool InventoryItem::IsPoisoned() const
{
    return CheckItemStatus(mStatus, ItemStatus::Poisoned);
}

bool InventoryItem::IsMoney() const
{
    return mItemIndex == ItemIndex{0x35}
        || mItemIndex == ItemIndex{0x36};
}

bool InventoryItem::IsKey() const
{
    return IsItemType(ItemType::Key) || mItemIndex == BAK::sPicklock;
}

void InventoryItem::SetActivated(bool state)
{
    mStatus = SetItemStatus(mStatus, ItemStatus::Activated, state);
}

void InventoryItem::SetEquipped(bool state)
{
    mStatus = SetItemStatus(mStatus, ItemStatus::Equipped, state);
}

void InventoryItem::SetRepairable(bool state)
{
    mStatus = SetItemStatus(mStatus, ItemStatus::Repairable, state);
}

void InventoryItem::SetBroken(bool state)
{
    mStatus = SetItemStatus(mStatus, ItemStatus::Broken, state);
}

void InventoryItem::SetCondition(unsigned condition)
{
    mCondition = condition;
}

void InventoryItem::SetQuantity(unsigned quantity)
{
    ASSERT(!IsStackable() || (IsStackable() && quantity <= GetObject().mStackSize));
    mCondition = quantity;
}

bool InventoryItem::HasFlag(ItemFlags flag) const
{
    return CheckBitSet(GetObject().mFlags, flag);
}

bool InventoryItem::IsConditionBased() const
{
    return HasFlag(ItemFlags::ConditionBased);
}

bool InventoryItem::IsChargeBased() const
{
    return HasFlag(ItemFlags::ChargeBased);
}

bool InventoryItem::IsStackable() const
{
    return HasFlag(ItemFlags::Stackable);
}

bool InventoryItem::IsQuantityBased() const
{
    return HasFlag(ItemFlags::QuantityBased);
}

bool InventoryItem::IsConsumable() const
{
    return HasFlag(ItemFlags::Consumable);
}

bool InventoryItem::IsMagicUserOnly() const
{
    return HasFlag(ItemFlags::MagicalItem);
}

bool InventoryItem::IsSwordsmanUserOnly() const
{
    return HasFlag(ItemFlags::SwordsmanItem);
}

bool InventoryItem::IsSkillModifier() const
{
    return GetObject().mModifierMask != 0;
}

bool InventoryItem::IsItemType(BAK::ItemType type) const
{
    return GetObject().mType == type;
}

bool InventoryItem::IsItemModifier() const
{
    return IsItemType(ItemType::WeaponOil)
        || IsItemType(ItemType::ArmorOil)
        || IsItemType(BAK::ItemType::SpecialOil);
}

bool InventoryItem::IsModifiableBy(ItemType itemType)
{
    return (itemType == ItemType::WeaponOil && IsItemType(ItemType::Sword))
        || (itemType == ItemType::ArmorOil && IsItemType(ItemType::Armor))
        || (itemType == ItemType::SpecialOil 
                && (IsItemType(ItemType::Armor)
                    || IsItemType(ItemType::Sword)));
}

bool InventoryItem::IsRepairItem() const
{
    return IsItemType(BAK::ItemType::Tool);
}

bool InventoryItem::DisplayCondition() const
{
    return IsConditionBased();
}

bool InventoryItem::DisplayNumber() const
{
    return IsConditionBased() 
        || IsStackable()
        || IsChargeBased()
        || IsQuantityBased()
        || IsKey();
}

bool InventoryItem::HasModifier(Modifier mod) const
{
    return CheckBitSet(mModifiers, mod);
}

void InventoryItem::ClearTemporaryModifiers()
{
    mStatus = mStatus & 0b0111'1111;
    mModifiers = mModifiers & 0b1110'0000;
}

void InventoryItem::SetStatusAndModifierFromMask(std::uint16_t mask)
{
    mStatus |= (mask & 0xff);
    mModifiers |= ((mask >> 8) & 0xff);
}

void InventoryItem::SetModifier(Modifier mod)
{
    mModifiers = SetBit(mModifiers, static_cast<std::uint8_t>(mod), true);
}

void InventoryItem::UnsetModifier(Modifier mod)
{
    mModifiers = SetBit(mModifiers, static_cast<std::uint8_t>(mod), false);
}

std::vector<Modifier> InventoryItem::GetModifiers() const
{
    auto mods = std::vector<Modifier>{};
    for (unsigned i = 0; i < 8; i++)
        if (CheckBitSet(mModifiers, i))
            mods.emplace_back(static_cast<Modifier>(i));
    return mods;
}

std::pair<unsigned, unsigned> InventoryItem::GetItemUseSound() const
{
    return std::make_pair(GetObject().mUseSound, GetObject().mSoundPlayTimes);
}

std::ostream& operator<<(std::ostream& os, const InventoryItem& i)
{
    os << i.GetObject().mName << " #" << i.GetItemIndex() << " pct/qty: " << 
        +i.GetCondition() << " status: " << +i.GetStatus() << " mods: [" << i.GetModifiers() 
        << "] IsEquipped: " << i.IsEquipped() << "\n";
    return os;
}

InventoryItem InventoryItemFactory::MakeItem(
    ItemIndex itemIndex,
    std::uint8_t quantity,
    std::uint8_t status,
    std::uint8_t modifiers)
{
    const auto& objects = ObjectIndex::Get();

    return InventoryItem{
        &objects.GetObject(itemIndex),
        itemIndex,
        quantity,
        status,
        modifiers};
}

InventoryItem InventoryItemFactory::MakeItem(
    ItemIndex itemIndex,
    std::uint8_t quantity)
{
    return MakeItem(
        itemIndex,
        quantity,
        0,
        0);
}

}
