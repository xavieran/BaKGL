#include "logger.hpp"

#include "worldFactory.hpp"

#include "FileManager.h"
#include "FileBuffer.h"

#include "Palette.h"
#include "PaletteResource.h"

#include "glm.hpp"

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

    auto textures = BAK::TextureStore{zoneLabel, pal};
    auto zoneItems = BAK::ZoneItemStore{zoneLabel, textures};

    logger.Info() << "Loading world tile:" << tileX << tileY << std::endl;

    auto world = BAK::World{zoneItems, x, y};

    for (const auto& item : world.GetItems())
    {
        const auto& name = item.GetZoneItem().GetName();
        if (name.substr(0, 4) == "tree") continue;
        logger.Info() << "Item: " << name << " loc: " << item.GetLocation() << std::endl;
    }

    return 0;
}

