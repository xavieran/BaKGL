#include "bak/scene.hpp"

#include "bak/logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    std::string ttmFile{argv[1]};

    logger.Info() << "Loading TTM:" << ttmFile << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer(ttmFile);
    BAK::LoadScenes(fb);

    return 0;
}

