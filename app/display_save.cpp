#include "bak/gameData.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::SetLogTime(false);
    Logging::LogState::Disable("LoadSpells");
    Logging::LogState::Disable("LoadSpellDoc");
    Logging::LogState::Disable("Symbol");
    Logging::LogState::Disable("CreateFileBuffer");
    
    std::string saveFile{argv[1]};

    logger.Info() << "Loading save:" << saveFile << std::endl;

    BAK::GameData gameData(saveFile);
    logger.Info() << "SaveName: " << gameData.mName << "\n";
    logger.Info() << "Tile: " << std::hex << gameData.mLocation.mTile << std::dec <<  " " << gameData.mLocation.mTile << "\n";
    logger.Info() << "Location: " << std::hex << gameData.mLocation.mLocation << std::dec << "\n";
    logger.Info() << "Location: " << gameData.mLocation.mLocation << "\n";

    //auto containers = gameData.LoadContainers(4);
    auto containers = gameData.LoadShops();
    logger.Info() << "Containers: \n";
    for (auto& con : containers)
    {
        logger.Info() << "  Container: " << con << "\n";
    }

    return 0;
}

