#pragma once

#include "bak/IContainer.hpp"

#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <ostream>

namespace BAK {

struct ContainerWorldLocationTag {};

struct ContainerWorldLocation
{
    ZoneNumber mZone;
    std::array<std::uint8_t, 3> mUnknown;
    glm::vec<2, unsigned> mLocation;
};

std::ostream& operator<<(std::ostream&, const ContainerWorldLocation&);

struct ContainerGDSLocationTag{};

struct ContainerGDSLocation
{
    std::array<std::uint8_t, 4> mUnknown;
    HotspotRef mLocation;
};

std::ostream& operator<<(std::ostream&, const ContainerGDSLocation&);

using ContainerLocation = std::variant<
    ContainerWorldLocation,
    ContainerGDSLocation>;

std::ostream& operator<<(std::ostream&, const ContainerLocation&);

class ContainerHeader
{
public:
    ContainerHeader(ContainerWorldLocationTag, FileBuffer& fb);
    ContainerHeader(ContainerGDSLocationTag, FileBuffer& fb);

    ZoneNumber GetZone() const;
    GamePosition GetPosition() const;
    HotspotRef GetHotspotRef() const;

//private:
    ContainerLocation mLocation;

    std::uint8_t mLocationType; // no idea
    std::uint8_t mItems;
    std::uint8_t mCapacity;
    std::uint8_t mContainerType;
    std::optional<ShopStats> mShopData;
};

std::ostream& operator<<(std::ostream&, const ContainerHeader&);

class Container : public IContainer
{
public:
    Container(
        unsigned address,
        unsigned number,
        unsigned numberItems,
        unsigned capacity,
        ContainerType type,
        Target dialog,
        glm::vec<2, unsigned> location,
        std::optional<ShopStats> shopData,
        Inventory&& inventory);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }
    bool CanAddItem(const InventoryItem& item) const override
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

    ContainerType GetContainerType() const override
    {
        return mType;
    }

    const ShopStats& GetShopData() const override
    {
        ASSERT(mShopData);
        return *mShopData;
    }

    unsigned mAddress;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    ContainerType mType;
    Target mDialog;
    glm::vec<2, unsigned> mLocation;
    std::optional<ShopStats> mShopData;
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
        ContainerType type,
        std::optional<ShopStats> shopData,
        Inventory&& inventory);
 
    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool CanAddItem(const InventoryItem& item) const override
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

    ContainerType GetContainerType() const override
    {
        return mType;
    }

    const ShopStats& GetShopData() const override
    {
        ASSERT(mShopData);
        return *mShopData;
    }

    BAK::HotspotRef mGdsScene;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    ContainerType mType;
    std::optional<ShopStats> mShopData;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream& os, const GDSContainer& i);



}
