#pragma once

#include "bak/resourceNames.hpp"
#include "bak/types.hpp"
#include "bak/zoneReference.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "graphics/glm.hpp"

#include "bak/fileBuffer.hpp"

namespace BAK {

class FMapXY
{
public:
    static constexpr auto sFile = "FMAP_XY.DAT";

    FMapXY()
    {
        auto fb = FileBufferFactory::Get().Get().CreateDataBuffer(sFile);
        unsigned i = 0;

        for (unsigned zone = 0; zone < 12; zone++)
        {
            mTiles.emplace_back(LoadZoneRef(ZoneLabel{zone + 1}.GetZoneReference()));
            unsigned i = 0;
            const auto nTiles = fb.GetUint16LE();
            Logging::LogDebug("FMAP") << "Tiles in zone: " << (zone + 1) << ": " << nTiles << std::endl;
            auto& tileCoords = mTileCoords.emplace_back();
            for (unsigned i = 0; i < nTiles; i++)
            {
                const auto x = fb.GetUint16LE();
                const auto y = fb.GetUint16LE();
                tileCoords.emplace_back(x, y);
                Logging::LogDebug("FMAP") << zone + 1 << " " << i 
                    << " " << mTiles.back()[i] << " ( " << x << ", " << y << ")\n";
            }
        }
    }

    glm::vec2 GetTileCoords(ZoneNumber zone, glm::uvec2 tile)
    {
        const auto& tiles = mTiles[zone.mValue - 1];
        const auto it = std::find(tiles.begin(), tiles.end(), tile);
        // Obviously this should not happen, but since I haven't implemented clipping it can
        if (it == tiles.end())
            return glm::vec2{0, 0};
        //ASSERT(it != tiles.end());
        const auto index = std::distance(tiles.begin(), it);
        return mTileCoords[zone.mValue - 1][index];
    }

private:
    std::vector<std::vector<glm::uvec2>> mTiles;
    std::vector<std::vector<glm::vec2>> mTileCoords;
};

struct Town
{
    Town(
        std::string name,
        std::uint16_t type,
        glm::vec2 coord)
    :
        mName{name},
        mType{type},
        mCoord{coord}
    {}

    std::string mName;
    std::uint16_t mType;
    glm::vec2 mCoord;
};

class FMapTowns
{
public:
    static constexpr auto sFile = "FMAP_TWN.DAT";

    FMapTowns()
    {
        auto fb = FileBufferFactory::Get().Get().CreateDataBuffer(sFile);
        const auto x1 = fb.GetUint16LE();
        const auto x2 = fb.GetUint16LE();
        const auto y1 = fb.GetUint16LE();
        const auto y2 = fb.GetUint16LE();
        const auto z1 = fb.GetUint16LE();

        Logging::LogDebug("FMAP_TWN") << x1 << " " << x2
            << " " << y1 << " " << y2 << " " << z1 << "\n";

        for (unsigned i = 0; i < 33; i++)
        {
            const auto type = fb.GetUint16LE();
            const auto town = fb.GetString();
            const auto x = fb.GetUint16LE();
            const auto y = fb.GetUint16LE();
            Logging::LogDebug("FMAP_TWN") << i << " " << town 
                << " ( " << x << ", " << y << ") tp: "
                << type << "\n";
            mTowns.emplace_back(town, type, glm::vec2{x, y});
        }
    }

    const auto& GetTowns() const { return mTowns; }

private:
    std::vector<Town> mTowns;
};

}
