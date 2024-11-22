#pragma once

#include "bak/IContainer.hpp"

#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/shop.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/lock.hpp"
#include "bak/inventory.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"
#include "com/bits.hpp"

#include <optional>
#include <ostream>
#include <variant>

namespace BAK {

// Bit indices
enum class ContainerProperty
{
    HasLock      = 0, // 0x1
    HasDialog    = 1, // 0x2
    HasShop      = 2, // 0x4
    HasEncounter = 3, // 0x8
    HasTime      = 4, // 0x10
    HasDoor      = 5  // 0x20
};

struct ContainerWorldLocationTag {};

struct ContainerWorldLocation
{
    ZoneNumber mZone;
    std::uint8_t mChapterRange;
    std::uint8_t mModel;
    std::uint8_t mUnknown;
    glm::uvec2 mLocation;

    unsigned GetFrom() const;
    unsigned GetTo() const;
};

std::ostream& operator<<(std::ostream&, const ContainerWorldLocation&);

struct ContainerGDSLocationTag{};

struct ContainerGDSLocation
{
    std::array<std::uint8_t, 4> mUnknown;
    HotspotRef mLocation;
};

std::ostream& operator<<(std::ostream&, const ContainerGDSLocation&);

struct ContainerCombatLocationTag{};

struct ContainerCombatLocation
{
    std::array<std::uint8_t, 4> mUnknown;
    std::uint32_t mCombatant;
    std::uint32_t mCombat;
};

std::ostream& operator<<(std::ostream&, const ContainerCombatLocation&);


using ContainerLocation = std::variant<
    ContainerWorldLocation,
    ContainerGDSLocation,
    ContainerCombatLocation>;

std::ostream& operator<<(std::ostream&, const ContainerLocation&);

class ContainerHeader
{
public:
    ContainerHeader();
    ContainerHeader(ContainerWorldLocationTag, FileBuffer& fb);
    ContainerHeader(ContainerGDSLocationTag, FileBuffer& fb);
    ContainerHeader(ContainerCombatLocationTag, FileBuffer& fb);

    ZoneNumber GetZone() const;
    GamePosition GetPosition() const;
    HotspotRef GetHotspotRef() const;
    unsigned GetCombatNumber() const;
    unsigned GetCombatantNumber() const;
    unsigned GetModel() const;
    bool PresentInChapter(Chapter) const;

    bool HasLock() const { return CheckBitSet(mFlags, ContainerProperty::HasLock); }
    bool HasDialog() const { return CheckBitSet(mFlags, ContainerProperty::HasDialog); }
    bool HasShop() const { return CheckBitSet(mFlags, ContainerProperty::HasShop); }
    bool HasEncounter() const { return CheckBitSet(mFlags, ContainerProperty::HasEncounter); }
    bool HasTime() const { return CheckBitSet(mFlags, ContainerProperty::HasTime); }
    bool HasDoor() const { return CheckBitSet(mFlags, ContainerProperty::HasDoor); }
    bool HasInventory() const { return mCapacity != 0; }

    std::uint32_t GetAddress() const { return mAddress; }

//private:

    std::uint32_t mAddress;
    ContainerLocation mLocation;

    std::uint8_t mLocationType; // no idea
    std::uint8_t mItems;
    std::uint8_t mCapacity;
    std::uint8_t mFlags;
};

std::ostream& operator<<(std::ostream&, const ContainerHeader&);

struct ContainerEncounter
{
    std::uint16_t mRequireEventFlag;
    std::uint16_t mSetEventFlag;
    std::optional<HotspotRef> mHotspotRef;
    std::optional<GamePosition> mEncounterPos;
};

std::ostream& operator<<(std::ostream&, const ContainerEncounter&);

struct ContainerDialog
{
    std::uint8_t mContextVar;
    std::uint8_t mDialogOrder;
    Target mDialog;
};

std::ostream& operator<<(std::ostream&, const ContainerDialog&);

class GenericContainer final : public IContainer {
public:
    static constexpr auto sTypicalShopBuffer = 6;

    GenericContainer(
        ContainerHeader header,
        std::optional<LockStats> lock,
        std::optional<Door> door,
        std::optional<ContainerDialog> dialog,
        std::optional<ShopStats> shop,
        std::optional<ContainerEncounter> encounter,
        std::optional<Time> lastAccessed,
        Inventory&& inventory)
    :
        mHeader{header},
        mLock{lock},
        mDoor{door},
        mDialog{dialog},
        mShop{shop},
        mEncounter{encounter},
        mLastAccessed{lastAccessed},
        mInventory{std::move(inventory)}
    {
    }

    GenericContainer(GenericContainer&&) noexcept                 = default;
    GenericContainer& operator=(GenericContainer&&) noexcept      = default;
    GenericContainer(const GenericContainer&) noexcept            = default;
    GenericContainer& operator=(const GenericContainer&) noexcept = default;

    const ContainerHeader& GetHeader() const { return mHeader; }

    bool HasLock() const { return bool{mLock}; }
    LockStats& GetLock() override { ASSERT(mLock); return *mLock; }
    const LockStats& GetLock() const { ASSERT(mLock); return *mLock; }

    bool HasDialog() const { return bool{mDialog}; }
    const ContainerDialog& GetDialog() const { ASSERT(mDialog); return *mDialog; }
    ContainerDialog& GetDialog() { ASSERT(mDialog); return *mDialog; }

    bool HasDoor() const { return bool{mDoor}; }
    DoorIndex GetDoor() const { ASSERT(mDoor); return mDoor->mDoorIndex; }
    DoorIndex GetDoor() { ASSERT(mDoor); return mDoor->mDoorIndex; }

    bool HasShop() const { return bool{mShop}; }
    ShopStats& GetShop() override { ASSERT(mShop); return *mShop; }
    const ShopStats& GetShop() const override { ASSERT(mShop); return *mShop; }

    bool HasEncounter() const { return bool{mEncounter}; }
    ContainerEncounter& GetEncounter() { ASSERT(mEncounter); return *mEncounter; }
    const ContainerEncounter& GetEncounter() const { ASSERT(mEncounter); return *mEncounter; }

    bool HasLastAccessed() const { return bool{mLastAccessed}; }
    Time& GetLastAccessed() { ASSERT(mLastAccessed); return *mLastAccessed; }
    const Time& GetLastAccessed() const { ASSERT(mLastAccessed); return *mLastAccessed; }

    bool HasInventory() const { return bool{mHeader.mCapacity != 0}; }

    Inventory& GetInventory() override { return mInventory; }
    const Inventory& GetInventory() const override { return mInventory; }

    bool CanAddItem(const InventoryItem& item) const override
    {
        return mInventory.CanAddContainer(item);
    }

    bool GiveItem(const InventoryItem& item) override
    {
        if (mShop && !mInventory.CanAddContainer(item))
        {
            // Remove the earliest added item
            mInventory.RemoveItem(BAK::InventoryIndex(mInventory.GetCapacity() - sTypicalShopBuffer));
        }

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

private:
    ContainerHeader mHeader;
    std::optional<LockStats> mLock;
    std::optional<Door> mDoor;
    std::optional<ContainerDialog> mDialog;
    std::optional<ShopStats> mShop;
    std::optional<ContainerEncounter> mEncounter;
    std::optional<Time> mLastAccessed;
    Inventory mInventory;
};

template <typename HeaderTag>
GenericContainer LoadGenericContainer(FileBuffer& fb);

std::ostream& operator<<(std::ostream&, const GenericContainer&);

}
