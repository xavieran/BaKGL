#include "bak/keyContainer.hpp"

#include "bak/inventoryItem.hpp"

namespace BAK {

KeyContainer::KeyContainer(
    Inventory&& inventory)
:
    mInventory{std::move(inventory)}
{}

Inventory& KeyContainer::GetInventory() { return mInventory; }
const Inventory& KeyContainer::GetInventory() const { return mInventory; }

bool KeyContainer::CanAddItem(const InventoryItem& item) const
{
    return item.IsItemType(ItemType::Key);
}

bool KeyContainer::GiveItem(const InventoryItem& item)
{
    ASSERT(item.IsKey());
    auto it = mInventory.FindItem(item);
    if (it != mInventory.GetItems().end())
        it->SetQuantity(it->GetQuantity() + 1);
    else
        mInventory.AddItem(item);
    return true;
}

bool KeyContainer::RemoveItem(const InventoryItem& item)
{
    auto it = mInventory.FindItem(item);
    if (it != mInventory.GetItems().end())
    {
        ASSERT(it->GetQuantity() > 0);
        it->SetQuantity(it->GetQuantity() - 1);
        if (it->GetQuantity() == 0)
        {
            mInventory.RemoveItem(
                InventoryIndex{static_cast<unsigned>(
                    std::distance(
                        mInventory.GetItems().begin(),it))});
        }
    }
    else
    {
        return false;
    }

    return true;
}

ContainerType KeyContainer::GetContainerType() const
{
    return ContainerType::Key;
}

ShopStats& KeyContainer::GetShop() { ASSERT(false); return *reinterpret_cast<ShopStats*>(this);}
const ShopStats& KeyContainer::GetShop() const { ASSERT(false); return *reinterpret_cast<const ShopStats*>(this);}
LockStats& KeyContainer::GetLock() { ASSERT(false); return *reinterpret_cast<LockStats*>(this); }

std::ostream& operator<<(std::ostream& os, const KeyContainer& i)
{
    os << "KeyContainer {" << i.mInventory << "}";
    return os;
}

}
