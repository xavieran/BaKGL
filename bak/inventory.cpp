#include "bak/inventory.hpp"

#include "bak/inventoryItem.hpp"
#include "bak/file/fileBuffer.hpp"

#include "com/assert.hpp"

#include <algorithm>
#include <numeric>

namespace BAK {

Inventory::Inventory(
    unsigned capacity)
:
    mCapacity{capacity},
    mItems{}
{}

Inventory::Inventory(
    unsigned capacity,
    std::vector<InventoryItem>&& items)
:
    mCapacity{capacity},
    mItems{std::move(items)}
{}

const std::vector<InventoryItem>& Inventory::GetItems() const { return mItems; }
std::vector<InventoryItem>& Inventory::GetItems() { return mItems; }

std::size_t Inventory::GetCapacity() const { return mCapacity; }
std::size_t Inventory::GetNumberItems() const { return mItems.size(); }

const InventoryItem& Inventory::GetAtIndex(InventoryIndex i) const
{
    ASSERT(i.mValue < mItems.size());
    return mItems[i.mValue];
}

InventoryItem& Inventory::GetAtIndex(InventoryIndex i)
{
    ASSERT(i.mValue < mItems.size());
    return mItems[i.mValue];
}

std::vector<InventoryItem>::const_iterator Inventory::FindItem(const InventoryItem& item) const
{
    return std::find_if(
        mItems.begin(), mItems.end(),
        [&item](const auto& elem){
            return elem.GetItemIndex() == item.GetItemIndex();
        });
}

std::vector<InventoryItem>::iterator Inventory::FindItem(const InventoryItem& item)
{
    return std::find_if(
        mItems.begin(), mItems.end(),
        [&item](const auto& elem){
            return elem.GetItemIndex() == item.GetItemIndex();
        });
}

std::optional<InventoryIndex> Inventory::GetIndexFromIt(std::vector<InventoryItem>::iterator it)
{
    if (it == mItems.end())
        return std::nullopt;
    else
        return std::make_optional(
            static_cast<InventoryIndex>(std::distance(mItems.begin(), it)));
}

std::optional<InventoryIndex> Inventory::GetIndexFromIt(std::vector<InventoryItem>::const_iterator it) const
{
    if (it == mItems.end())
        return std::nullopt;
    else
        return std::make_optional(
            static_cast<InventoryIndex>(std::distance(mItems.cbegin(), it)));
}

std::vector<InventoryItem>::const_iterator Inventory::FindStack(const InventoryItem& item) const
{
    ASSERT(item.IsStackable() || item.IsChargeBased());
    auto items = std::vector<
        std::pair<
            std::size_t,
            std::reference_wrapper<const InventoryItem>>>{};

    for (std::size_t i = 0; i < mItems.size(); i++)
    {
        if (mItems[i].GetItemIndex() == item.GetItemIndex())
            items.emplace_back(i, std::ref(mItems[i]));
    }

    auto it = std::min_element(items.begin(), items.end(),
        [](const auto& l, const auto& r){
            return (std::get<1>(l).get().GetQuantity() < std::get<1>(r).get().GetQuantity());
        });

    // If we didn't find an incomplete stack, return a complete one
    if (it == items.end())
        return FindItem(item);
    else
        return std::next(mItems.begin(), it->first);
}

// Search for a stackable item prioritising incomplete stacks
std::vector<InventoryItem>::iterator Inventory::FindStack(const InventoryItem& item)
{
    // Is there a better way?
    auto cit = static_cast<const Inventory*>(this)->FindStack(item);
    return std::next(
        mItems.begin(),
        std::distance(mItems.cbegin(), cit));
}

std::vector<InventoryItem>::const_iterator Inventory::FindEquipped(BAK::ItemType slot) const
{
    return std::find_if(
        mItems.begin(), mItems.end(),
        [&slot](const auto& elem){
            return elem.IsItemType(slot) && elem.IsEquipped();
        });
}

std::vector<InventoryItem>::iterator Inventory::FindEquipped(BAK::ItemType slot)
{
    return std::find_if(
        mItems.begin(), mItems.end(),
        [&slot](const auto& elem){
            return elem.IsItemType(slot) && elem.IsEquipped();
        });
}

std::vector<InventoryItem>::const_iterator Inventory::FindItemType(BAK::ItemType slot) const
{
    return std::find_if(
        mItems.begin(), mItems.end(),
        [&slot](const auto& elem){
            return elem.IsItemType(slot);
        });
}

std::vector<InventoryItem>::iterator Inventory::FindItemType(BAK::ItemType slot)
{
    return std::find_if(
        mItems.begin(), mItems.end(),
        [&slot](const auto& elem){
            return elem.IsItemType(slot);
        });
}

unsigned Inventory::CalculateModifiers(SkillType skill) const
{
    unsigned mods = 0;
    for (const auto& item : mItems)
    {
        if ((item.GetObject().mModifierMask 
            & (1 << static_cast<unsigned>(skill))) != 0)
        {
            mods += item.GetObject().mModifier;
        }
    }
    return mods;
}

void Inventory::CopyFrom(Inventory& other)
{
    for (const auto& item : other.GetItems())
    {
        auto newItem = item;
        newItem.SetActivated(false);
        newItem.SetEquipped(false);
        AddItem(newItem);
    }
}

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
    // This is an annoying hack because the game gives characters inventory
    // size of 24, rather than 20 + 8 (items + equipped)
    const auto currentQuantity = GetSpaceUsed() + 4;
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

bool Inventory::RemoveItem(BAK::InventoryIndex index, unsigned quantity)
{
    ASSERT(index.mValue < mItems.size());
    if (index.mValue < mItems.size())
    {
        auto& item = mItems[index.mValue];
        ASSERT(quantity <= item.GetQuantity());
        item.SetQuantity(item.GetQuantity() - quantity);
        if (item.GetQuantity() == 0)
        {
            mItems.erase(mItems.begin() + index.mValue);
        }
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

bool Inventory::ReplaceItem(BAK::InventoryIndex index, BAK::InventoryItem item)
{
    ASSERT(index.mValue < mItems.size());
    if (index.mValue < mItems.size())
    {
        mItems[index.mValue] = item;
        return true;
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
            // The game actually does not enforce this at all...
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
        const auto& object = ObjectIndex::Get().GetObject(item);
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
