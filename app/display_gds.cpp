#include "bak/hotspot.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("LoadScenes");
    Logging::LogState::Disable("LoadSceneIndices");
    
    std::string gdsFile{argv[1]};

    logger.Info() << "Loading gds:" << gdsFile << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer(gdsFile);
    BAK::SceneHotspots sceneHotspots{std::move(fb)};

    return 0;
}

