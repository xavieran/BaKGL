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
            return (elem.GetItemIndex() == item.GetItemIndex())
                && (elem.GetQuantity() != elem.GetObject().mStackSize);
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
            return item.GetItemIndex() == elem.GetItemIndex();
        });

    if (it != mItems.end() && item.IsStackable())
    {
        return it->GetCondition() >= item.GetCondition();
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
        const auto additionalQuantity = std::min(item.GetQuantity(), amountToStack);
        it->SetQuantity(it->GetQuantity() + additionalQuantity);

        if (item.GetQuantity() > amountToStack)
        {
            auto newItem = item;
            newItem.SetCondition(item.GetQuantity() - amountToStack);
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
    if ((item.IsStackable() || item.IsChargeBased())
        && HaveItem(item))
    {
        unsigned remainingToRemove = item.GetQuantity();
        auto it = FindStack(item);
        do 
        {
            ASSERT(it != mItems.end());
            const auto amountToRemove = std::min(
                remainingToRemove,
                it->GetQuantity());

            it->SetQuantity(it->GetQuantity() - amountToRemove);
            remainingToRemove -= amountToRemove;

            if (it->GetQuantity() == 0)
                mItems.erase(it);

        } while (remainingToRemove > 0
            && (it = FindStack(item)) != mItems.end());

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
    os << "Inventory {\n";
    for (unsigned i = 0; i < inventory.GetNumberItems(); i++)
    {
        os << "#" << i << " -> " << inventory.GetAtIndex(InventoryIndex{i}) << "\n";
    }
    os << "}";
    return os;
}

Inventory LoadInventory(FileBuffer& fb, unsigned itemCount, unsigned capacity)
{
    auto items = std::vector<InventoryItem>{};
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
