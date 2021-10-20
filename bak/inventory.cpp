#include "bak/inventory.hpp"

#include "com/assert.hpp"

#include <iostream>

namespace BAK {


bool Inventory::HasIncompleteStack(const InventoryItem& item) const
{
    const auto it = std::find_if(
        mItems.begin(), mItems.end(),
        [&item](const auto& elem){
            const auto stackSize = elem.GetObject().mStackSize;
            return (elem.mItemIndex == item.mItemIndex)
                && (elem.mCondition != elem.GetObject().mStackSize);
        });
    return it != mItems.end();
}

bool Inventory::CanAdd(const InventoryItem& item) const
{
    // Keys can always be added
    if (item.GetObject().mType == ItemType::Key)
        return true;

    const auto currentQuantity = std::accumulate(
        mItems.begin(), mItems.end(),
        0,
        [](const auto sum, const auto& elem) -> unsigned {
            if (!elem.IsEquipped())
                return sum + elem.GetObject().mImageSize;
            return sum;
        });

    return (currentQuantity + item.GetObject().mImageSize) <= sMaxInventorySize;
}

bool Inventory::HaveWeaponEquipped() const
{
    for (const auto& item : mItems)
    {
        const auto itemType = item.GetObject().mType;
        if ((itemType == BAK::ItemType::Sword
            || itemType == BAK::ItemType::Staff)
            && item.IsEquipped())
            return true;
    }
    return false;
}

bool Inventory::HaveItem(const InventoryItem& item) const
{
    const auto it = std::find_if(
        mItems.begin(), mItems.end(),
        [item](const auto& elem){
            return item.mItemIndex == elem.mItemIndex;
        });

    if (it != mItems.end() && item.IsStackable())
    {
        return it->mCondition >= item.mCondition;
    }

    return it != mItems.end();
}

void Inventory::AddItem(const InventoryItem& item)
{
    if (item.IsStackable() && HasIncompleteStack(item))
    {
        auto it = FindIncompleteStack(item);
        ASSERT(it != mItems.end());
        const auto amountToStack = item.GetObject().mStackSize - it->mCondition;
        it->mCondition += std::min(
            item.mCondition,
            static_cast<std::uint8_t>(amountToStack));

        if (item.mCondition > amountToStack)
            mItems.emplace_back(
                InventoryItemFactory::MakeItem(
                    item.mItemIndex,
                    item.mCondition - amountToStack));
    }
    else
    {
        mItems.emplace_back(item);
    }
}

bool Inventory::RemoveItem(BAK::InventoryIndex item)
{
    ASSERT(item.mValue < mItems.size());
    if (item.mValue < mItems.size())
    {
        mItems.erase(mItems.begin() + item.mValue);
        return true;
    }
    return false;
}

bool Inventory::RemoveItem(const InventoryItem& item)
{
    if (item.IsStackable() && HaveItem(item))
    {
        // First try to remove from an incomplete stack
        auto it = FindIncompleteStack(item);
        if (it == mItems.end())
            it = FindItem(item);

        ASSERT(it != mItems.end());
        it->mCondition -= item.mCondition;
        if (it->mCondition == 0)
            mItems.erase(it);

        return true;
    }
    else
    {
        auto it = FindItem(item);
        if (it != mItems.end())
        {
            mItems.erase(it);
            return true;
        }
    }

    return false;
}

std::ostream& operator<<(std::ostream& os, const Inventory& inventory)
{
    for (unsigned i = 0; i < inventory.GetItems().size(); i++)
    {
        os << i << " >> " << inventory.GetItems()[i] << "\n";
    }
    return os;
}

}
