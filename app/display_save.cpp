#include "bak/gameData.hpp"

extern "C" {
#include "com/getopt.h"
}

#include "com/string.hpp"

#include "com/logger.hpp"

struct Options
{
    bool shops{};
    std::array<bool, 13> zones{};

    bool party{};
    bool skill_affectors{};
    bool time_state{};

    bool combat_inventories{};
    bool combat_grid{};
    bool combat_world{};
    bool combat_stats{};
    bool combat_click{};
    bool combat_lists{};

    std::string saveFile{};
};

Options Parse(int argc, char** argv)
{
    Options values{};

    struct option options[] = {
        {"help", no_argument,       0, 'h'},
        {"shop", no_argument, 0, 's'},
        {"zone", required_argument, 0, 'z'},
        {"party", required_argument, 0, 'p'},
        {"skill_affectors", required_argument, 0, 'k'},
        {"time_state", required_argument, 0, 't'},
        {"combat_inventories", no_argument, 0, 'c'},
        {"combat_grid", no_argument, 0, 'g'},
        {"combat_world", no_argument, 0, 'w'},
        {"combat_stats", no_argument, 0, 'S'},
        {"combat_click", no_argument, 0, 'C'},
        {"combat_lists", no_argument, 0, 'l'},
    };
    int optionIndex = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hsz:pktcgwSCl", options, &optionIndex)) != -1)
    {   
        if (opt == 'h')
        {
            exit(0);
        }
        else if (opt == 's')
        {
            values.shops = true;
        }
        else if (opt == 'z')
        {
            auto zones = SplitString(",", std::string{optarg});
            for (auto z : zones)
            {
                values.zones[std::atoi(z.c_str())] = true;
            }
        }
        else if (opt == 'p')
        {
            values.party = true;
        }
        else if (opt == 'k')
        {
            values.skill_affectors = true;
        }
        else if (opt == 't')
        {
            values.time_state = true;
        }
        else if (opt == 'c')
        {
            values.combat_inventories = true;
        }
        else if (opt == 'g')
        {
            values.combat_grid = true;
        }
        else if (opt == 'w')
        {
            values.combat_world = true;
        }
        else if (opt == 'S')
        {
            values.combat_stats = true;
        }
        else if (opt == 'C')
        {
            values.combat_click = true;
        }
        else if (opt == 'l')
        {
            values.combat_lists = true;
        }
    }

    if (optind < argc)
    {
        values.saveFile = argv[optind];
    }

    return values;
}

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::SetLogTime(false);
    Logging::LogState::Disable("LoadSpells");
    Logging::LogState::Disable("LoadSpellDoc");
    Logging::LogState::Disable("Symbol");
    Logging::LogState::Disable("CreateFileBuffer");
    
    const auto options = Parse(argc, argv);

    logger.Info() << "Loading save:" << options.saveFile << std::endl;

    BAK::GameData gameData(options.saveFile);
    logger.Info() << "SaveName: " << gameData.mName << "\n";
    logger.Info() << "Tile: " << std::hex << gameData.mLocation.mTile << std::dec <<  " " << gameData.mLocation.mTile << "\n";
    logger.Info() << "MapLocation: " << gameData.mMapLocation << "\n";
    logger.Info() << "Location: " << gameData.mLocation.mLocation << "\n";
    logger.Info() << "Time: " << gameData.mTime;

    if (options.party)
    {
        auto party = gameData.LoadParty();
        logger.Info() << "Party: " << party << "\n";
    }

    if (options.skill_affectors)
    {
        for (unsigned i = 0; i < 6; i++)
        {
            logger.Info() << "SkillAffectors for character: " << i
                << " Affectors: " << gameData.GetCharacterSkillAffectors(BAK::CharIndex{i}) << "\n";
        }
    }

    if (options.time_state)
    {
        logger.Info() << "TimeExpiringState: " << gameData.LoadTimeExpiringState() << "\n";
    }

    for (unsigned i = 0; i < 13; i++)
    {
        if (options.zones[i])
        {
            auto containers = gameData.LoadContainers(i);
            logger.Info() << "Containers(" << i << ")\n";
            for (auto& con : containers)
            {
                logger.Info() << "  " << con << "\n";
            }
        }
    }

    if (options.shops)
    {
        logger.Info() << "Shops\n";
        auto containers = gameData.LoadShops();
        for (auto& con : containers)
        {
            logger.Info() << "  " << con << "\n";
        }
    }

    if (options.combat_inventories)
    {
        auto containers = gameData.LoadCombatInventories();
        logger.Info() << "Combat Inventories\n";
        for (auto& con : containers)
        {
            logger.Info() << "  " << con << "\n";
        }
    }

    if (options.combat_grid)
    {
        auto cgl = gameData.LoadCombatGridLocations();
        logger.Info() << "Combat Grid Locations\n";
        for (unsigned i = 0; i < cgl.size(); i++)
        {
            logger.Info() << "  #" << i << " " << cgl[i] << "\n";
        }
    }

    if (options.combat_world)
    {
        logger.Info() << "CombatWorldLocations: \n";
        auto cwl = gameData.LoadCombatWorldLocations();
        for (unsigned i = 0; i < cwl.size(); i++)
        {
            logger.Info() << "  #" << i << " " << cwl[i] << "\n";
        }
    }

    if (options.combat_lists)
    {
        logger.Info() << "CombatEntityLists: \n";
        auto cel = gameData.LoadCombatEntityLists();
        for (unsigned i = 0; i < cel.size(); i++)
        {
            logger.Info() << "  #" << i << " " << cel[i] << "\n";
        }
    }

    return 0;
}

