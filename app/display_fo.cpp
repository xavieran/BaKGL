#include "src/fixedObject.hpp"

#include "src/logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("DialogStore");
    
    std::string zone{argv[1]};

    logger.Info() << "Loading fixed objects from zone: " << zone << std::endl;

    BAK::LoadFixedObjects(std::atoi(zone.c_str()));

    return 0;
}

