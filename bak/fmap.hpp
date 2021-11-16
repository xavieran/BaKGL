#pragma once

#include "bak/resourceNames.hpp"
#include "bak/zoneReference.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include "xbak/FileBuffer.h"

namespace BAK {

class FMapXY
{
public:
    static constexpr auto sFile = "FMAP_XY.DAT";

    FMapXY()
    {
        auto fb = FileBufferFactory::CreateFileBuffer(sFile);
        unsigned i = 0;

        for (unsigned zone = 0; zone < 12; zone++)
        {
            const auto tiles = LoadZoneRef(ZoneLabel{zone + 1}.GetZoneReference());
            unsigned i = 0;
            const auto nTiles = fb.GetUint16LE();
            Logging::LogDebug("FMAP") << "Tiles in zone: " << (zone + 1) << ": " << nTiles << std::endl;
            for (unsigned i = 0; i < nTiles; i++)
            {
                const auto x = fb.GetUint16LE();
                const auto y = fb.GetUint16LE();
                Logging::LogDebug("FMAP") << zone + 1 << " " << i 
                    << " " << tiles[i] << " ( " << x << ", " << y << ")\n";
            }
        }
    }
};

class FMapTowns
{
public:
    static constexpr auto sFile = "FMAP_TWN.DAT";

    FMapTowns()
    {
        auto fb = FileBufferFactory::CreateFileBuffer(sFile);
        const auto x1 = fb.GetUint16LE();
        const auto x2 = fb.GetUint16LE();
        const auto y1 = fb.GetUint16LE();
        const auto y2 = fb.GetUint16LE();
        const auto z1 = fb.GetUint16LE();

        Logging::LogDebug("FMAP") << x1 << " " << x2
            << " " << y1 << " " << y2 << " " << z1 << "\n";

        for (unsigned i = 0; i < 33; i++)
        {
            const auto type = fb.GetUint16LE();
            const auto town = fb.GetString();
            const auto x = fb.GetUint16LE();
            const auto y = fb.GetUint16LE();
            Logging::LogDebug("FMAP") << i << " " << town 
                << " ( " << x << ", " << y << ") tp: "
                << type << "\n";
        }
    }

};

}
