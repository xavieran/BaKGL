#pragma once

#include "bak/IContainer.hpp"

#include "bak/dialog.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <ostream>

namespace BAK {

class Container : public IContainer
{
public:
    Container(
        unsigned address,
        unsigned number,
        unsigned numberItems,
        unsigned capacity,
        unsigned type,
        Target dialog,
        glm::vec<2, unsigned> location,
        Inventory&& inventory);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }
    bool CanAddItem(const InventoryItem& item) override
    {
        return mInventory.CanAddContainer(item);
    }

    bool GiveItem(const InventoryItem& item) override
    {
        mInventory.AddItem(item);
        return true;
    }

    bool RemoveItem(const InventoryItem& item) override
    {
        mInventory.RemoveItem(item);
        return true;
    }
    unsigned mAddress;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    Target mDialog;
    glm::vec<2, unsigned> mLocation;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const Container& i);

class GDSContainer : public IContainer
{
public:
    GDSContainer(
        BAK::HotspotRef gdsScene,
        unsigned number,
        unsigned numberItems,
        unsigned capacity,
        unsigned type,
        Inventory&& inventory);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool CanAddItem(const InventoryItem& item) override
    {
        return mInventory.CanAddContainer(item);
    }

    bool GiveItem(const InventoryItem& item) override
    {
        mInventory.AddItem(item);
        return true;
    }

    bool RemoveItem(const InventoryItem& item) override
    {
        mInventory.RemoveItem(item);
        return true;
    }

    BAK::HotspotRef mGdsScene;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const GDSContainer& i);



}
