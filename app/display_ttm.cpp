#include "bak/scene.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    std::string adsFile{argv[1]};
    std::string ttmFile{argv[2]};

    logger.Info() << "Loading ADS and TTM:" << adsFile << " " << ttmFile << std::endl;

    {
        auto fb = FileBufferFactory::CreateFileBuffer(adsFile);
        BAK::LoadSceneIndices(fb);
    }

    {
        auto fb = FileBufferFactory::CreateFileBuffer(ttmFile);
        BAK::LoadScenes(fb);
    }

    return 0;
}

