#include "bak/hotspot.hpp"
#include "bak/dialog.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    
    std::string gdsFile{argv[1]};

    logger.Info() << "Loading gds:" << gdsFile << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer(gdsFile);
    BAK::SceneHotspots sceneHotspots{};
    sceneHotspots.Load(fb);

    return 0;
}

