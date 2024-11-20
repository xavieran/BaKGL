#pragma once

#include "bak/types.hpp"

#include <iostream>

namespace BAK {

struct GameObject;
enum class ItemType;
enum class Modifier;

enum class ItemStatus : std::uint8_t
{
    Activated  = 1,
    Broken     = 4,
    Repairable = 5,
    Equipped   = 6,
    Poisoned   = 7
};

enum class ItemFlags : std::uint16_t
{
    Consumable     =  4, // 0x0010
    MagicalItem    =  7, // 0x0080
    Combat         =  8, // 0x0100 // ??
    SwordsmanItem  =  9, // 0x0200 // ??
    NonCombatItem  = 10, // 0x0400 // ??
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

    const GameObject& GetObject() const;

    ItemIndex GetItemIndex() const;
    unsigned GetQuantity() const;
    SpellIndex GetSpell() const;
    unsigned GetCondition() const;
    std::uint8_t GetStatus() const;
    std::uint8_t GetModifierMask() const;

    bool IsActivated() const;
    bool IsEquipped() const;
    bool IsBroken() const;
    bool IsRepairable() const;
	bool IsRepairableByShop() const;
    bool IsPoisoned() const;
    bool IsMoney() const;
    bool IsKey() const;

    bool HasFlag(ItemFlags flag) const;
    bool IsConditionBased() const;
    bool IsChargeBased() const;
    bool IsStackable() const;
    bool IsQuantityBased() const;
    bool IsConsumable() const;
    bool IsMagicUserOnly() const;
    bool IsSwordsmanUserOnly() const;
    bool IsSkillModifier() const;
    bool IsItemType(ItemType type) const;
    bool IsItemModifier() const;
    bool IsModifiableBy(ItemType itemType);
    bool IsRepairItem() const;
    bool DisplayCondition() const;
    bool DisplayNumber() const;
    bool HasModifier(Modifier mod) const;

    void SetActivated(bool state);
    void SetEquipped(bool state);
    void SetRepairable(bool state);
    void SetBroken(bool state);
    void SetCondition(unsigned condition);
    void SetQuantity(unsigned quantity);

    void ClearTemporaryModifiers();
    void SetStatusAndModifierFromMask(std::uint16_t mask);
    void SetModifier(Modifier mod);
    void UnsetModifier(Modifier mod);
    std::vector<Modifier> GetModifiers() const;
    std::pair<unsigned, unsigned> GetItemUseSound() const;

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
        std::uint8_t modifiers);

    static InventoryItem MakeItem(
        ItemIndex itemIndex,
        std::uint8_t quantity);
};

}
