#include "bak/save/world.hpp"

#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include <sstream>

namespace BAK {

unsigned LoadChapter(FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sChapterOffset);
    auto chapter = fb.GetUint16LE();
    fb.Seek(0x64);
    auto chapterAgain = fb.GetUint16LE();
    assert(chapter == chapterAgain);
    return chapterAgain;
}

WorldClock LoadWorldTime(FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sTimeOffset);
    return WorldClock{
        Time{fb.GetUint32LE()},
        Time{fb.GetUint32LE()}};
}

MapLocation LoadMapLocation(FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sMapPositionOffset);
    auto posX = fb.GetUint16LE();
    auto posY = fb.GetUint16LE();
    auto heading = fb.GetUint16LE();
    auto mapLocation = MapLocation{{posX, posY}, heading};
    const auto& logger = Logging::LogState::GetLogger("LoadMapLocation");
    logger.Info() << mapLocation << "\n";
    return mapLocation;
}

Location LoadLocation(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadLocation");
    fb.Seek(SaveOffsets::sLocationOffset);

    unsigned zone = fb.GetUint8();
    ASSERT(zone <= 12);
    logger.Info() << "LOADED: Zone:" << zone << std::endl;

    unsigned xtile = fb.GetUint8();
    unsigned ytile = fb.GetUint8();
    unsigned xpos = fb.GetUint32LE();
    unsigned ypos = fb.GetUint32LE();

    logger.Info() << "Unknown: " << fb.GetArray<5>() << "\n";
    std::uint16_t heading = fb.GetUint16LE();

    logger.Info() << "Tile: " << xtile << "," << ytile << std::endl;
    logger.Info() << "Pos: " << xpos << "," << ypos << std::endl;
    logger.Info() << "Heading: " << heading << std::endl;

    return Location{
        ZoneNumber{zone},
        {xtile, ytile},
        GamePositionAndHeading{
            GamePosition{xpos, ypos},
            heading}
    };
}

std::vector<TimeExpiringState> LoadTimeExpiringState(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadTimeExpiringState");
    fb.Seek(SaveOffsets::sTimeExpiringEventRecordOffset);
    auto storage = std::vector<TimeExpiringState>{};
    auto stateCount = fb.GetUint16LE();
    for (unsigned i = 0; i < stateCount; i++)
    {
        auto type = fb.GetUint8();
        auto flag = fb.GetUint8();
        auto data= fb.GetUint16LE();
        auto time = Time{fb.GetUint32LE()};
        storage.emplace_back(TimeExpiringState{ExpiringStateType{type}, flag, data, time});
        logger.Spam() << storage.back() << "\n";
    }
    return storage;
}

SpellState LoadSpells(FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sActiveSpells);
    return SpellState{fb.GetUint16LE()};
}

void LoadChapterOffsetP(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger("LoadChapterOffsetP");
    constexpr unsigned chapterOffsetsStart = 0x11a3;
    fb.Seek(chapterOffsetsStart);

    logger.Spam() << "Chapter Offsets Start @"
        << std::hex << chapterOffsetsStart << std::dec << std::endl;

    for (unsigned i = 0; i < 10; i++)
    {
        std::stringstream ss{};
        ss << "Chapter #" << i << " : " << fb.GetUint16LE();
        for (unsigned i = 0; i < 5; i++)
        {
            unsigned addr = fb.GetUint32LE();
            ss << " a: " << std::hex << addr << std::dec;
        }
        logger.Spam() << ss.str() << std::endl;
    }

    logger.Spam() << "Chapter Offsets End @"
        << std::hex << fb.Tell() << std::dec << std::endl;
}

}
