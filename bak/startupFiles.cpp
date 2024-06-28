#include "bak/startupFiles.hpp"
#include "bak/resourceNames.hpp"

#include "bak/coordinates.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

namespace BAK {

ChapterStartLocation LoadChapterStartLocation(Chapter chapter)
{
    // Yes these are hardcoded in the game.
    static const std::vector<MapLocation> chapterStartLocations{
        MapLocation{{116, 75}, 18},
        MapLocation{{168, 147}, 18},
        MapLocation{{256, 114}, 2},
        MapLocation{{167, 24}, 26},
        MapLocation{{234, 52}, 2},
        MapLocation{{168, 148}, 2},
        MapLocation{{184, 92}, 18},
        MapLocation{{0, 0}, 0}, // timirianya
        MapLocation{{203, 128}, 2}
    };
    std::stringstream ss{};
    ss << "CHAP";
    ss << chapter.mValue;
    ss << ".DAT";

    auto fb = FileBufferFactory::Get().CreateDataBuffer(ss.str());

    unsigned fileChapter = fb.GetUint16LE();
    fb.Skip(4); // this is likely party gold. But it's always zero...
    Time timeChange = BAK::Time{fb.GetUint32LE()};
    fb.Skip(6);
    unsigned zone = fb.GetUint8();
    unsigned tileX = fb.GetUint8();
    unsigned tileY = fb.GetUint8();
    unsigned cellX = fb.GetUint8();
    unsigned cellY = fb.GetUint8();
    std::uint16_t heading = fb.GetUint16LE() / 0x100;

    auto pos = MakeGamePositionFromTileAndCell(glm::uvec2{tileX, tileY}, glm::uvec2{cellX, cellY});

    return ChapterStartLocation{
        chapterStartLocations[chapter.mValue - 1],
        Location{
            ZoneNumber{zone},
            {tileX, tileY},
            GamePositionAndHeading{
                pos,
                heading}},
        timeChange};
}

void LoadDetect()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("DETECT.DAT");

    for (unsigned i = 0; i < 2; i++)
    {
        std::vector<std::uint32_t> items{};
        for (unsigned i = 0; i < 43; i++)
        {
            items.emplace_back(fb.GetUint32LE());
            Logging::LogDebug(__FUNCTION__) << i << " - " << items.back() << "\n";
        }
    }
    Logging::LogDebug(__FUNCTION__) << "Remaining: " << fb.GetBytesLeft() << "\n";
}

void LoadConfig()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("KRONDOR.CFG");
    auto movementSize = fb.GetUint8();
    auto turnSize = fb.GetUint8();
    auto detail = fb.GetUint8();
    auto textSpeed = fb.GetUint8();
    auto soundConfiguration = fb.GetUint8();
}

void LoadFilter()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("FILTER.DAT");

    std::vector<std::uint32_t> items{};
    for (unsigned i = 0; i < 4; i++)
    {
        for (unsigned i = 0; i < 43; i++)
        {
            items.emplace_back(fb.GetUint32LE());
            Logging::LogDebug(__FUNCTION__) << i << " - " << items.back() << "\n";
        }
    }
    Logging::LogDebug(__FUNCTION__) << "Remaining: " << fb.GetBytesLeft() << "\n";
}

void LoadZoneDefDat(ZoneNumber zone)
{
    auto zoneLabel = ZoneLabel{zone.mValue};
    auto fb = FileBufferFactory::Get().CreateDataBuffer(zoneLabel.GetZoneDefault());
    Logging::LogDebug(__FUNCTION__) << "Zone: " << zone.mValue << "\n";

    const auto zoneType = fb.GetUint16LE();
    const auto threeDParam = fb.GetUint16LE();
    const auto playerPos_fieldA = fb.GetUint32LE();
    const auto playerPos_fieldE = fb.GetUint16LE();
    const auto horizonDisplayType = fb.GetUint16LE();
    const auto groundType = fb.GetUint8();
    const auto groundHeight = fb.GetUint8();
    const auto minMapZoom = fb.GetUint32LE();
    const auto unknown8 = fb.GetUint32LE();
    const auto maxMapZoom = fb.GetUint32LE();
    const auto mapZoomRate = fb.GetUint32LE();
    const auto unknown11 = fb.GetUint16LE();
    const auto unknown12 = fb.GetUint16LE();
    const auto unknown13 = fb.GetUint32LE();
    const auto unknown14 = fb.GetUint32LE();
    const auto unknown15 = fb.GetUint16LE();
    const auto unknown16 = fb.GetUint32LE();
    const auto unknown17 = fb.GetUint32LE();

    Logging::LogDebug(__FUNCTION__) << "ZoneType: " << zoneType << " 3dParam? " << threeDParam <<"\n";
    Logging::LogDebug(__FUNCTION__) << "PlayerPos A: " << playerPos_fieldA << " PlayerPos E: " << playerPos_fieldE << "\n";
    Logging::LogDebug(__FUNCTION__) << "HorizonAndGroundType: " << horizonDisplayType << "\n";
    Logging::LogDebug(__FUNCTION__) << "GroundType: " << +groundType << " GroundHeight: " << +groundHeight << "\n";
    Logging::LogDebug(__FUNCTION__) << " minMapZoom: " << minMapZoom << " ? " << unknown8 << "\n";
    Logging::LogDebug(__FUNCTION__) << " maxMapZoom: " << maxMapZoom << " mapZoomRate: " << mapZoomRate << "\n";
    Logging::LogDebug(__FUNCTION__) << " 11: " << unknown11 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 12: " << unknown12 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 13: " << unknown13 << " 14: " << unknown14 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 15: " << unknown15 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 16: " << unknown16 << " 17: " << unknown17 << "\n";
    Logging::LogDebug(__FUNCTION__) << "Remaining: " << fb.GetBytesLeft() << "\n";
}

ZoneMap LoadZoneMap(ZoneNumber zone)
{
    auto zoneLabel = ZoneLabel{zone.mValue};
    auto fb = FileBufferFactory::Get().CreateDataBuffer(zoneLabel.GetZoneMap());
    Logging::LogDebug(__FUNCTION__) << "Zone: " << zone.mValue << "\n";

    ZoneMap map{};
    for (unsigned i = 0; i < 0x190; i++)
    {
        map[i] = fb.GetUint8();
    }

    std::stringstream ss{};
    for (unsigned x = 0; x < 50; x++)
    {
        for (unsigned y = 0; y < 50; y++)
        {
            auto val = GetMapDatTileValue(x, y, map);
            if (val > 0)
            {
                ss << "#";;
            }
            else
            {
                ss << ".";
            }
        }
        ss << "\n";
    }
    Logging::LogDebug(__FUNCTION__) << "ZoneMap: \n" << ss.str() << "\n";
    return map;
}

unsigned GetMapDatTileValue(
    unsigned x,
    unsigned y,
    const ZoneMap& mapDat)
{
    auto mapVal = mapDat[(x << 3) + (y >> 3)];
    auto mask = 1 << (y % 8);
    return mapVal & mask;
}

void LoadZoneDat(ZoneNumber zone)
{
    auto zoneLabel = ZoneLabel{zone.mValue};
    auto fb = FileBufferFactory::Get().CreateDataBuffer(zoneLabel.GetZoneDat());
    Logging::LogDebug(__FUNCTION__) << "Zone: " << zone << "\n";

    auto word0 = fb.GetUint16LE();
    auto word1 = fb.GetUint16LE();
    auto word2 = fb.GetUint16LE();
    auto word3 = fb.GetUint16LE();

    Logging::LogDebug(__FUNCTION__) << "Data: " << word0 << " " << word1 << " " << word2 << " " << word3 << "\n";
    Logging::LogDebug(__FUNCTION__) << "Remaining:" << fb.GetBytesLeft() << "\n";
}

}
