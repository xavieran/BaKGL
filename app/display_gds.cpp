#include "src/scene.hpp"
#include "src/dialog.hpp"

#include "src/logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("DialogStore");
    
    std::string gdsFile{argv[1]};

    logger.Info() << "Loading gds:" << gdsFile << std::endl;

    auto fb = FileBufferFactory::CreateFileBuffer(gdsFile);
    BAK::Scene scene{};
    scene.Load(fb);

    return 0;
}

