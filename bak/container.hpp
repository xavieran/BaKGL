#pragma once

#include "bak/IContainer.hpp"

#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/inventory.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/bits.hpp"

#include <variant>
#include <ostream>

namespace BAK {

// Bit indices
enum class ContainerProperty
{
    HasLock      = 0,
    HasDialog    = 1,
    HasShop      = 2,
    HasEncounter = 3,
    HasTime      = 4
};

struct ContainerWorldLocationTag {};

struct ContainerWorldLocation
{
    ZoneNumber mZone;
    std::array<std::uint8_t, 3> mUnknown;
    glm::uvec2 mLocation;
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

    bool HasLock() const { return CheckBitSet(mFlags, ContainerProperty::HasLock); }
    bool HasDialog() const { return CheckBitSet(mFlags, ContainerProperty::HasDialog); }
    bool HasShop() const { return CheckBitSet(mFlags, ContainerProperty::HasShop); }
    bool HasEncounter() const { return CheckBitSet(mFlags, ContainerProperty::HasEncounter); }
    bool HasTime() const { return CheckBitSet(mFlags, ContainerProperty::HasTime); }
    bool HasInventory() const { return mCapacity != 0; }

//private:
    ContainerLocation mLocation;

    std::uint8_t mLocationType; // no idea
    std::uint8_t mItems;
    std::uint8_t mCapacity;
    std::uint8_t mFlags;
};

std::ostream& operator<<(std::ostream&, const ContainerHeader&);

struct ContainerEncounter
{
    unsigned mRequireEventFlag;
    unsigned mSetEventFlag;
    std::optional<HotspotRef> mHotspotRef;
    std::optional<GamePosition> mEncounterPos;
};

std::ostream& operator<<(std::ostream&, const ContainerEncounter&);

struct ContainerDialog
{
    unsigned mUnknown;
    Target mDialog;
};

std::ostream& operator<<(std::ostream&, const ContainerDialog&);

class GenericContainer : public IContainer
{
public:

    GenericContainer(
        ContainerHeader header,
        std::optional<LockStats> lock,
        std::optional<ContainerDialog> dialog,
        std::optional<ShopStats> shop,
        std::optional<ContainerEncounter> encounter,
        std::optional<Time> lastAccessed,
        Inventory&& inventory)
    :
        mHeader{header},
        mLock{lock},
        mDialog{dialog},
        mShop{shop},
        mEncounter{encounter},
        mLastAccessed{lastAccessed},
        mInventory{std::move(inventory)}
    {
    }


    ContainerHeader mHeader;
    std::optional<LockStats> mLock;
    std::optional<ContainerDialog> mDialog;
    std::optional<ShopStats> mShop;
    std::optional<ContainerEncounter> mEncounter;
    std::optional<Time> mLastAccessed;
    Inventory mInventory;

    bool HasLock() const { return bool{mLock}; }
    LockStats& GetLock() override { ASSERT(mLock); return *mLock; }

    bool HasDialog() const { return bool{mDialog}; }
    ContainerDialog& GetDialog() { ASSERT(mDialog); return *mDialog; }

    bool HasShop() const { return bool{mShop}; }
    ShopStats& GetShop() override { ASSERT(mShop); return *mShop; }

    bool HasEncounter() const { return bool{mEncounter}; }
    ContainerEncounter& GetEncounter() { ASSERT(mEncounter); return *mEncounter; }
    bool HasLastAccessed() const { return bool{mLastAccessed}; }
    Time& GetLastAccessed() { ASSERT(mLastAccessed); return *mLastAccessed; }

    bool HasInventory() const { return bool{mHeader.mCapacity != 0}; }

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
        return static_cast<ContainerType>(mHeader.mFlags);
    }
};

GenericContainer LoadGenericContainer(FileBuffer& fb, bool isGdsLocation);

std::ostream& operator<<(std::ostream&, const GenericContainer&);

}
