#include "gameData.hpp"

#include "logger.hpp"

#include "FileManager.h"
#include "FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    std::string saveFile{argv[1]};

    logger.Info() << "Loading save:" << saveFile << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer(saveFile);
    BAK::GameData gameData(fb);

    return 0;
}

