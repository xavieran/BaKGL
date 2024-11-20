#include "com/logger.hpp"

#include "bak/encounter/encounter.hpp"

#include "bak/zoneReference.hpp"
#include "bak/palette.hpp"
#include "bak/worldFactory.hpp"

#include "graphics/glm.hpp"

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
    
    auto textures = BAK::ZoneTextureStore{zoneLabel};
    auto zoneItems = BAK::ZoneItemStore{zoneLabel, textures};
    const auto tiles = BAK::LoadZoneRef(zoneLabel.GetZoneReference());
    unsigned i;
    for (i = 0; i < tiles.size(); i++)
        if (tiles[i].x == x && tiles[i].y == y)
            break;

    logger.Info() << "Loading world tile:" << tileX << tileY << std::endl;

    const auto ef = BAK::Encounter::EncounterFactory{};
    auto world = BAK::World{zoneItems, ef, x, y, i};

    for (const auto& item : world.GetItems())
    {
        const auto& name = item.GetZoneItem().GetName();
        if (name.substr(0, 4) == "tree") continue;
        logger.Info() << "Item: " << name << " loc: " << item.GetLocation() << std::endl;
    }

    constexpr auto chapter = 1;
    logger.Info() << "Encounters: " << world.GetEncounters(BAK::Chapter{chapter}).size() << "\n";
    for (const auto& encounter : world.GetEncounters(BAK::Chapter{chapter}))
    {
        logger.Info() << "Encounter: " << encounter << "\n";
    }

    return 0;
}

