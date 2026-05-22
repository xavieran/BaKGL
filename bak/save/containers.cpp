#include "bak/save/containers.hpp"

#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/inventory.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

namespace BAK {

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

void Save(const Inventory& inv, FileBuffer& fb)
{
    const auto itemCount = inv.GetNumberItems();
    const auto capacity = inv.GetCapacity();

    unsigned i = 0;
    for (; i < itemCount; i++)
    {
        const auto& item = inv.GetAtIndex(InventoryIndex{i});
        fb.PutUint8(item.GetItemIndex().mValue);
        fb.PutUint8(item.GetCondition());
        fb.PutUint8(item.GetStatus());
        fb.PutUint8(item.GetModifierMask());
    }

    for (; i < capacity; i++)
        fb.Skip(4);
}

void Save(const GenericContainer& gc, FileBuffer& fb)
{
    // Write container header
    fb.Seek(gc.GetHeader().GetAddress());
    fb.Skip(12); // Skip mLocation, that won't change
    fb.Skip(1); // Skip mLocationType
    fb.PutUint8(gc.GetInventory().GetNumberItems());
    fb.Skip(1);
    fb.Skip(1);

    Save(gc.GetInventory(), fb);

    if (gc.HasLock())
    {
        const auto& lock = gc.GetLock();
        fb.PutUint8(lock.mLockFlag);
        fb.PutUint8(lock.mRating);
        fb.PutUint8(lock.mFairyChestIndex);
        fb.PutUint8(lock.mTrapDamage);
    }

    if (gc.HasDoor())
    {
        fb.Skip(2); // No need to write anything for doors
    }

    if (gc.HasDialog())
        fb.Skip(6); // Don't need to write anything for dialog

    if (gc.HasShop())
    {
        const auto& shop = gc.GetShop();
        fb.PutUint8(shop.mTempleNumber);
        fb.PutUint8(shop.mSellFactor);
        fb.PutUint8(shop.mMaxDiscount);
        fb.PutUint8(shop.mBuyFactor);
        fb.PutUint8(shop.mHaggleDifficulty);
        fb.PutUint8(shop.mHaggleAnnoyanceFactor);
        fb.PutUint8(shop.mBardingSkill);
        fb.PutUint8(shop.mBardingReward);
        fb.PutUint8(shop.mBardingMaxReward);
        fb.PutUint8(shop.mUnknown);
        fb.PutUint8(shop.mInnSleepTilHour);
        fb.PutUint8(shop.mInnCost);
        fb.PutUint8(shop.mRepairTypes);
        fb.PutUint8(shop.mRepairFactor);
        fb.PutUint16LE(shop.mCategories);
    }

    if (gc.HasEncounter())
        fb.Skip(9); // Don't need to write anything for encounter

    if (gc.HasLastAccessed())
        fb.PutUint32LE(gc.GetLastAccessed().mTime);
}

std::vector<GenericContainer> LoadShops(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadShops");
    fb.Seek(SaveOffsets::sShopsOffset);
    auto shops = std::vector<GenericContainer>{};

    for (unsigned i = 0; i < SaveOffsets::sShopsCount; i++)
    {
        const unsigned address = fb.Tell();
        logger.Spam() << " Container: " << i
            << " addr: " << std::hex << address << std::dec << std::endl;
        auto container = LoadGenericContainer<ContainerGDSLocationTag>(fb);
        shops.emplace_back(std::move(container));
        logger.Spam() << shops.back() << "\n";
    }

    return shops;
}

std::vector<GenericContainer> LoadContainers(FileBuffer& fb, unsigned zone)
{
    const auto& logger = Logging::LogState::GetLogger("LoadContainers");
    logger.Spam() << "Loading containers for Z: " << zone << "\n";
    std::vector<GenericContainer> containers{};

    ASSERT(zone < SaveOffsets::sZoneContainerOffsets.size());
    const auto [offset, count] = SaveOffsets::sZoneContainerOffsets[zone];
    fb.Seek(offset);

    for (unsigned j = 0; j < count; j++)
    {
        const unsigned address = fb.Tell();
        logger.Spam() << " Container: " << j
            << " addr: " << std::hex << address << std::dec << std::endl;
        auto container = LoadGenericContainer<ContainerWorldLocationTag>(fb);
        containers.emplace_back(std::move(container));
        logger.Spam() << containers.back() << "\n";
    }

    return containers;
}

std::vector<GenericContainer> LoadCombatInventories(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadCombatInventories");
    logger.Spam() << "Loading Combat Inventories" << std::endl;
    fb.Seek(SaveOffsets::sCombatInventoryOffset);
    std::vector<GenericContainer> containers{};

    for (unsigned i = 0; i < SaveOffsets::sCombatInventoryCount; i++)
    {
        auto loc = fb.Tell();
        auto container = LoadGenericContainer<ContainerCombatLocationTag>(fb);
        containers.emplace_back(std::move(container));
        logger.Spam() << "CombatInventory #" << i << " @" << std::hex << loc << std::dec << " " << containers.back() << "\n";
    }

    return containers;
}

}
