#include "bak/gameData.hpp"
#include "bak/save/world.hpp"
#include "bak/save/party.hpp"
#include "bak/save/character.hpp"
#include "bak/save/combat.hpp"
#include "bak/save/containers.hpp"

#include "bak/coordinates.hpp"
#include "bak/fmap.hpp"

extern "C" {
#include "com/getopt.h"
}

#include "com/logger.hpp"
#include "com/ostream.hpp"
#include "com/path.hpp"

#include <iostream>
#include <string>

struct Options
{
    std::string mInputFile{};
    std::string mGameData{};
};

Options Parse(int argc, char** argv)
{
    Options values{};

    struct option options[] = {
        {"help", no_argument, 0, 'h'},
        {"game-data", required_argument, 0, 'g'},
    };
    int optionIndex = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hg:", options, &optionIndex)) != -1)
    {
        if (opt == 'h')
        {
            std::cout << "Resaves a BaK save file by re-serializing all state.\n";
            std::cout << "\t--game-data,-g  :: path to game data directory (needed for map tile lookup)\n";
            std::cout << "\t<savefile>      :: input .GAM save file to resave\n";
            exit(0);
        }
        else if (opt == 'g')
        {
            values.mGameData = optarg;
        }
    }

    if (optind < argc)
    {
        values.mInputFile = argv[optind];
    }

    return values;
}

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::SetLogTime(false);
    Logging::LogState::Disable("CreateFileBuffer");

    const auto options = Parse(argc, argv);

    if (options.mInputFile.empty())
    {
        logger.Error() << "No input file specified\n";
        return 1;
    }

    if (!options.mGameData.empty())
    {
        Paths::Get().SetBakDirectory(options.mGameData);
    }

    logger.Info() << "Loading save: " << options.mInputFile << std::endl;

    BAK::GameData gameData(options.mInputFile);
    auto& fb = gameData.GetFileBuffer();

    logger.Info() << "Save name: " << gameData.mName << std::endl;
    logger.Info() << "Chapter: " << gameData.mChapter << std::endl;

    auto gdsContainers = BAK::LoadShops(fb);
    auto combatContainers = BAK::LoadCombatInventories(fb);
    std::vector<std::vector<BAK::GenericContainer>> zoneContainers;
    for (unsigned i = 0; i < 13; i++)
    {
        zoneContainers.emplace_back(BAK::LoadContainers(fb, i));
    }
    auto timeExpiringState = BAK::LoadTimeExpiringState(fb);
    auto combatWorldLocations = BAK::LoadCombatWorldLocations(fb);
    auto spellState = BAK::LoadSpells(fb);

    // Re-serialize everything into the buffer (same order as GameState::SaveState)
    BAK::Save(gameData.mTime, fb);
    BAK::Save(gameData.mParty, fb);

    for (const auto& container : gdsContainers)
        BAK::Save(container, fb);

    for (const auto& container : combatContainers)
        BAK::Save(container, fb);

    for (const auto& zc : zoneContainers)
        for (const auto& container : zc)
            BAK::Save(container, fb);

    BAK::Save(timeExpiringState, fb);
    BAK::Save(spellState, fb);
    BAK::Save(gameData.mChapter, fb);

    if (!options.mGameData.empty())
    {
        BAK::FMapXY fmap;
        auto& location = gameData.mLocation;
        auto zone = BAK::ZoneNumber{location.mZone};
        auto tile = BAK::GetTile(location.mLocation.mPosition);
        auto mapLocation = BAK::MapLocation{
            fmap.GetTileCoords(zone, tile),
            BAK::HeadingToFullMapAngle(location.mLocation.mHeading)};
        BAK::Save(mapLocation, fb);
    }
    else
    {
        BAK::Save(gameData.mMapLocation, fb);
    }

    BAK::Save(gameData.mLocation, fb);
    BAK::Save(combatWorldLocations, fb);

    auto outputPath = options.mInputFile + ".new";
    gameData.Save(gameData.mName, outputPath);

    logger.Info() << "Wrote: " << outputPath << std::endl;

    return 0;
}
