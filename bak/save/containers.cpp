#include "bak/save/containers.hpp"

#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

namespace BAK {

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
