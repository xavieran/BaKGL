#include "bak/gameData.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Info);
    
    std::string saveFile{argv[1]};

    logger.Info() << "Loading save:" << saveFile << std::endl;

    BAK::GameData gameData(saveFile);
    logger.Info() << "SaveName: " << gameData.mName << "\n";
    logger.Info() << "Tile: " << std::hex << gameData.mLocation.mTile << std::dec <<  " " << gameData.mLocation.mTile << "\n";
    logger.Info() << "Location: " << std::hex << gameData.mLocation.mLocation << std::dec << "\n";
    logger.Info() << "Location: " << gameData.mLocation.mLocation << "\n";

    gameData.LoadCombatInventories();
    return 0;
}

