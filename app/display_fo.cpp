#include "bak/fixedObject.hpp"

#include "bak/monster.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    
    std::string zone{argv[1]};

    logger.Info() << "Loading fixed objects from zone: " << zone << std::endl;

    const auto obj = BAK::LoadFixedObjects(std::atoi(zone.c_str()));

    for (const auto& o : obj)
    {
        logger.Info() << o << "\n";
    }

    const auto mon = BAK::MonsterNames{};

    return 0;
}

