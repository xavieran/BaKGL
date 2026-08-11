#include "bak/save/underground.hpp"

#include "bak/file/fileBuffer.hpp"
#include "bak/save/saveOffsets.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

namespace BAK {

std::vector<TileVisibility> LoadUnderground(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::vector<TileVisibility> entries{};

    for (unsigned i = 0; i < sMaxUndergroundTiles; i++)
    {
        fb.Seek(SaveOffsets::sUndergroundBegin + i);
        auto zone = fb.GetUint8();
        fb.Seek(SaveOffsets::sUndergroundBegin + 1 * sMaxUndergroundTiles + i);
        auto tileX = fb.GetUint8();
        fb.Seek(SaveOffsets::sUndergroundBegin + 2 * sMaxUndergroundTiles + i);
        auto tileY = fb.GetUint8();
        fb.Seek(SaveOffsets::sUndergroundBegin + 3 * sMaxUndergroundTiles + i * sUndergroundObjectFields);
        auto visibilityFields = fb.GetArray<sUndergroundObjectFields>();

        std::array<bool, sUndergroundObjectFields * sBitsPerField> visibleObjects{};
        for (unsigned byte = 0; byte < sUndergroundObjectFields; byte++)
        {
            for (unsigned bit = 0; bit < sBitsPerField; bit++)
            {
                visibleObjects[byte * sBitsPerField + bit] = (visibilityFields[byte] >> bit) & 0x1;
            }
        }

        logger.Info() << "Entry #" << i << " z: " << +zone
            << " x: " << +tileX << " y: " << +tileY << "\n";
        logger.Info() << "Entry #" << i << " dump: " << std::hex << visibilityFields << std::dec << "\n";
        entries.emplace_back(zone, tileX, tileY, visibleObjects);
    }

    return entries;
}

void Save(const std::vector<TileVisibility>& entries, FileBuffer& fb)
{
    for (unsigned i = 0; i < sMaxUndergroundTiles; i++)
    {
        const auto& entry = entries[i];
        fb.Seek(SaveOffsets::sUndergroundBegin + i);
        fb.PutUint8(entry.mZone);
        fb.Seek(SaveOffsets::sUndergroundBegin + 1 * sMaxUndergroundTiles + i);
        fb.PutUint8(entry.mTileX);
        fb.Seek(SaveOffsets::sUndergroundBegin + 2 * sMaxUndergroundTiles + i);
        fb.PutUint8(entry.mTileY);
        fb.Seek(SaveOffsets::sUndergroundBegin + 3 * sMaxUndergroundTiles + i * sUndergroundObjectFields);
        for (unsigned byte = 0; byte < sUndergroundObjectFields; byte++)
        {
            unsigned char field = 0;
            for (unsigned bit = 0; bit < sBitsPerField; bit++)
            {
                field |= (entry.mVisibleObjects[byte * sBitsPerField + bit] ? 1 : 0) << bit;
            }
            fb.PutUint8(field);
        }
    }
}

}
