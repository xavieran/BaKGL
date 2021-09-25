#include "com/logger.hpp"

#include "graphics/glm.hpp"
#include "bak/worldFactory.hpp"
#include "bak/encounter/encounter.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"
#include "xbak/Palette.h"
#include "xbak/PaletteResource.h"


int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    if (argc != 4)
    {
        logger.Info() << "usage: display_tile <zone> <x> <y>" << std::endl;
        return 1;
    }
    auto zoneLabel = BAK::ZoneLabel{argv[1]};
    std::string tileX {argv[2]};
    std::string tileY {argv[3]};
    unsigned x = std::atoi(tileX.c_str());
    unsigned y = std::atoi(tileY.c_str());
    
    auto palz = std::make_unique<PaletteResource>();
    FileManager::GetInstance()->Load(palz.get(), zoneLabel.GetPalette());
    auto& pal = *palz->GetPalette();

    auto textures = BAK::ZoneTextureStore{zoneLabel, pal};
    auto zoneItems = BAK::ZoneItemStore{zoneLabel, textures};

    logger.Info() << "Loading world tile:" << tileX << tileY << std::endl;

    auto world = BAK::World{zoneItems, x, y};

    for (const auto& item : world.GetItems())
    {
        const auto& name = item.GetZoneItem().GetName();
        if (name.substr(0, 4) == "tree") continue;
        logger.Info() << "Item: " << name << " loc: " << item.GetLocation() << std::endl;
    }

    for (const auto& encounter : world.GetEncounters())
    {
        logger.Info() << "Encounter: " << encounter << "\n";
    }

    const auto encounterFactory = BAK::Encounter::EncounterFactory{};

    return 0;
}

