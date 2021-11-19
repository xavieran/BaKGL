#include "bak/inventory.hpp"

#include "com/assert.hpp"

#include <algorithm>
#include <numeric>

namespace BAK {

std::size_t Inventory::GetSpaceUsed() const
{
    return std::accumulate(
        mItems.begin(), mItems.end(),
        0,
        [](const auto sum, const auto& elem) -> unsigned {
            if (!elem.IsEquipped())
                return sum + elem.GetObject().mImageSize;
            return sum;
        });
}

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

std::size_t Inventory::CanAddCharacter(const InventoryItem& item) const
{
    const auto currentQuantity = GetSpaceUsed();
    const auto fits = (currentQuantity + item.GetObject().mImageSize) <= mCapacity;
    return CanAdd(fits, item);
}

std::size_t Inventory::CanAddContainer(const InventoryItem& item) const
{
    const auto fits = GetNumberItems() < mCapacity;
    return CanAdd(fits, item);
}

std::size_t Inventory::CanAdd(bool fits, const InventoryItem& item) const
{
    if (fits)
    {
        if (item.IsStackable())
            return item.GetQuantity();
        else
            return 1;
    }
    else if (item.IsStackable() && HasIncompleteStack(item))
    {
        ASSERT(item.GetQuantity() > 0);
        ASSERT(item.GetQuantity() <= item.GetObject().mStackSize);
        auto stack = FindStack(item);
        return std::min(
            item.GetObject().mStackSize - stack->GetQuantity(),
            item.GetQuantity());
    }

    return 0;
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
        ASSERT(item.GetQuantity() > 0);
        ASSERT(item.GetQuantity() <= item.GetObject().mStackSize);

        auto it = FindStack(item);
        ASSERT(it != mItems.end());

        const auto amountToStack = item.GetObject().mStackSize - it->GetQuantity();
        it->mCondition += std::min(
            item.mCondition,
            amountToStack);

        if (item.mCondition > amountToStack)
        {
            auto newItem = item;
            newItem.mCondition = item.mCondition - amountToStack;
            mItems.emplace_back(newItem);
         }
    }
    else
    {
        mItems.emplace_back(item);
    }

    CheckPostConditions();
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
        unsigned remainingToRemove = item.GetQuantity();
        auto it = FindStack(item);
        do 
        {
            ASSERT(it != mItems.end());
            const auto amountToRemove = std::min(
                remainingToRemove,
                it->GetQuantity());

            it->mCondition -= amountToRemove;
            remainingToRemove -= amountToRemove;

            if (it->GetQuantity() == 0)
                mItems.erase(it);

        } while ((it = FindStack(item)) != mItems.end()
            && remainingToRemove > 0);

        CheckPostConditions();
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

void Inventory::CheckPostConditions()
{
    for (const auto& item : mItems)
    {
        // We should never end up with a stack bigger than allowed
        if (item.IsStackable())
        {
            ASSERT(item.GetQuantity() <= item.GetObject().mStackSize);
            ASSERT(item.GetQuantity() > 0);
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Inventory& inventory)
{
    for (unsigned i = 0; i < inventory.GetItems().size(); i++)
    {
        os << i << " >> " << inventory.GetItems()[i] << "\n";
    }
    return os;
}

Inventory LoadItems(FileBuffer& fb, unsigned itemCount, unsigned capacity)
{
    std::vector<InventoryItem> items{};
    unsigned i;
    for (i = 0; i < itemCount; i++)
    {
        const auto item = ItemIndex{fb.GetUint8()};
        const auto& object = GetObjectIndex().GetObject(item);
        const auto condition = fb.GetUint8();
        const auto status = fb.GetUint8();
        const auto modifiers = fb.GetUint8();

        items.emplace_back(
            InventoryItemFactory::MakeItem(
                item,
                condition,
                status,
                modifiers));
    }

    for (; i < capacity; i++)
        fb.Skip(4);

    return Inventory{capacity, std::move(items)};
}

}
