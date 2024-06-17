#include "bak/startupFiles.hpp"
#include "bak/combat.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::SetLogTime(false);

    logger.Info() << "CHAPX.DAT\n";
    for (unsigned i = 1; i < 10; i++)
    {
        auto chapData = BAK::LoadChapterStartLocation(BAK::Chapter{i});
        logger.Info() << "Chapter " << i << " data: " << chapData.mLocation << "\n";
    }

    logger.Info() << "FILTER.DAT\n";
    BAK::LoadFilter();

    logger.Info() << "DETECT.DAT\n";
    BAK::LoadDetect();

    logger.Info() << "Z00DEF.DAT\n";
    for (unsigned i = 1; i < 13; i++)
    {
        BAK::LoadZoneDefDat(BAK::ZoneNumber{i});
    }

    logger.Info() << "Z00MAP.DAT\n";
    for (unsigned i = 1; i < 13; i++)
    {
        BAK::LoadZoneMap(BAK::ZoneNumber{i});
    }

    logger.Info() << "Z00.DAT\n";
    for (unsigned i = 1; i < 13; i++)
    {
        BAK::LoadZoneDat(BAK::ZoneNumber{i});
    }

    BAK::LoadP1Dat();

    return 0;
}
