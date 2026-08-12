#include "bak/save/underground.hpp"

#include "bak/file/fileBuffer.hpp"
#include "bak/save/saveOffsets.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

namespace BAK {

std::vector<TileVisibility> LoadTileVisibility(FileBuffer& fb)
{
    const auto& logger = Logging::LogState::GetLogger(__FUNCTION__);

    std::vector<TileVisibility> entries{};

    for (unsigned i = 0; i < sMaxUndergroundTiles; i++)
    {
        fb.Seek(SaveOffsets::sTileVisibilityRecords + i);
        auto zone = fb.GetUint8();
        fb.Seek(SaveOffsets::sTileVisibilityRecords + 1 * sMaxUndergroundTiles + i);
        auto tileX = fb.GetUint8();
        fb.Seek(SaveOffsets::sTileVisibilityRecords + 2 * sMaxUndergroundTiles + i);
        auto tileY = fb.GetUint8();
        fb.Seek(SaveOffsets::sTileVisibilityRecords + 3 * sMaxUndergroundTiles + i * sUndergroundObjectFields);
        auto visibilityFields = fb.GetArray<sUndergroundObjectFields>();

        std::array<bool, sUndergroundObjectFields * sBitsPerField> visibleObjects{};
        for (unsigned byte = 0; byte < sUndergroundObjectFields; byte++)
        {
            for (unsigned bit = 0; bit < sBitsPerField; bit++)
            {
                visibleObjects[byte * sBitsPerField + bit] = (visibilityFields[byte] >> bit) & 0x1;
            }
        }

        std::optional<TileLocation> location{};
        if (zone != sNoEntrySentinel)
        {
            location = TileLocation{zone, glm::uvec2{tileX, tileY}};
        }

        entries.emplace_back(location, visibleObjects);
    }

    return entries;
}

void Save(const std::vector<TileVisibility>& entries, FileBuffer& fb)
{
    for (unsigned i = 0; i < sMaxUndergroundTiles; i++)
    {
        const auto& entry = entries[i];
        fb.Seek(SaveOffsets::sTileVisibilityRecords + i);
        fb.PutUint8(entry.mTileLocation ? entry.mTileLocation->mZone : sNoEntrySentinel);
        fb.Seek(SaveOffsets::sTileVisibilityRecords + 1 * sMaxUndergroundTiles + i);
        fb.PutUint8(entry.mTileLocation ? entry.mTileLocation->mTile.x : sNoEntrySentinel);
        fb.Seek(SaveOffsets::sTileVisibilityRecords + 2 * sMaxUndergroundTiles + i);
        fb.PutUint8(entry.mTileLocation ? entry.mTileLocation->mTile.y : sNoEntrySentinel);
        fb.Seek(SaveOffsets::sTileVisibilityRecords + 3 * sMaxUndergroundTiles + i * sUndergroundObjectFields);
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

TileVisibility& EnsureTileVisibility(
    std::vector<TileVisibility>& entries,
    const TileLocation& location)
{
    for (auto& entry : entries)
    {
        if (!entry.mTileLocation)
        {
            entry.mTileLocation = location;
            entry.mVisibleObjects.fill(false);
            return entry;
        }

        if (entry.mTileLocation == location)
            return entry;
    }

    ASSERT(false && "No free tile visibility slot");
    return entries.back();
}

const TileVisibility* LookupTileVisibility(
    const std::vector<TileVisibility>& entries,
    const TileLocation& location)
{
    for (const auto& entry : entries)
    {
        if (entry.mTileLocation == location)
            return &entry;
    }
    return nullptr;
}

}
