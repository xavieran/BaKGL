#include "bak/zoneParams.hpp"

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/resourceNames.hpp"

#include "com/logger.hpp"

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdint>

namespace BAK {

ZoneViewport LoadZoneViewport()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("ZONE.DAT");
    const auto x = fb.GetUint16LE();
    const auto y = fb.GetUint16LE();
    const auto width = fb.GetUint16LE();
    const auto height = fb.GetUint16LE();

    return ZoneViewport{x, y, width, height};
}

glm::ivec4 ToGlViewport(ZoneViewport viewport, float scale)
{
    const auto x = std::round(viewport.mX * scale);
    const auto y = std::round((gNativeScreenHeight - (viewport.mY + viewport.mHeight)) * scale);
    const auto width = std::round(viewport.mWidth * scale);
    const auto height = std::round(viewport.mHeight * scale);

    return glm::ivec4{x, y, width, height};
}

ZoneDefaults LoadZoneDefDat(ZoneNumber zone)
{
    auto zoneLabel = ZoneLabel{zone.mValue};
    auto fb = FileBufferFactory::Get().CreateDataBuffer(zoneLabel.GetZoneDefault());
    Logging::LogDebug(__FUNCTION__) << "Zone: " << zone.mValue << "\n";

    const auto zoneType = fb.GetUint16LE();
    const auto focalLengthScale = fb.GetUint16LE();
    const auto defaultCameraHeight = fb.GetUint32LE();
    const auto playerPos_fieldE = fb.GetUint16LE();
    const auto horizonDisplayType = fb.GetUint16LE();
    const auto groundType = fb.GetUint8();
    const auto groundHeight = fb.GetUint8();
    const auto minMapZoom = fb.GetUint32LE();
    const auto defaultMapZoom = fb.GetUint32LE();
    const auto maxMapZoom = fb.GetUint32LE();
    const auto mapZoomRate = fb.GetUint32LE();
    const auto unknown11 = fb.GetUint16LE();
    const auto unknown12 = fb.GetUint16LE();
    const auto unknown13 = fb.GetUint32LE();
    const auto unknown14 = fb.GetUint32LE();
    const auto unknown15 = fb.GetUint16LE();
    const auto unknown16 = fb.GetUint32LE();
    const auto unknown17 = fb.GetUint32LE();

    Logging::LogDebug(__FUNCTION__) << "ZoneType: " << zoneType << " focalLengthScale " << focalLengthScale <<"\n";
    Logging::LogDebug(__FUNCTION__) << "DefaultCameraHeight: " << defaultCameraHeight
        << " PlayerPos E: " << playerPos_fieldE << "\n";
    Logging::LogDebug(__FUNCTION__) << "HorizonAndGroundType: " << horizonDisplayType << "\n";
    Logging::LogDebug(__FUNCTION__) << "GroundType: " << +groundType << " GroundHeight: " << +groundHeight << "\n";
    Logging::LogDebug(__FUNCTION__) << " minMapZoom: " << minMapZoom << " defaultMapZoom: " << defaultMapZoom << "\n";
    Logging::LogDebug(__FUNCTION__) << " maxMapZoom: " << maxMapZoom << " mapZoomRate: " << mapZoomRate << "\n";
    Logging::LogDebug(__FUNCTION__) << " 11: " << unknown11 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 12: " << unknown12 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 13: " << unknown13 << " 14: " << unknown14 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 15: " << unknown15 << "\n";
    Logging::LogDebug(__FUNCTION__) << " 16: " << unknown16 << " 17: " << unknown17 << "\n";
    Logging::LogDebug(__FUNCTION__) << "Remaining: " << fb.GetBytesLeft() << "\n";

    return ZoneDefaults{
        static_cast<float>(minMapZoom),
        static_cast<float>(defaultMapZoom),
        static_cast<float>(maxMapZoom),
        static_cast<float>(mapZoomRate),
        static_cast<int>(defaultCameraHeight),
        focalLengthScale};
}

CombatCamera LoadCombatCamera()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("START.DAT");
    const auto height = fb.GetUint16LE();
    const auto heightUnderground = fb.GetUint16LE();
    const auto viewAngle = fb.GetSint16LE();
    const auto viewAngleUnderground = fb.GetSint16LE();
    const auto combatGridCellSize = fb.GetUint16LE();
    const auto viewportX = fb.GetUint16LE();
    const auto viewportY = fb.GetUint16LE();
    const auto viewportWidth = fb.GetUint16LE();
    const auto viewportHeight = fb.GetUint16LE();
    const auto focalLengthScale = fb.GetUint16LE();

    return CombatCamera{
        static_cast<float>(height),
        static_cast<float>(heightUnderground),
        ToRadians(viewAngle),
        ToRadians(viewAngleUnderground),
        combatGridCellSize,
        ZoneViewport{viewportX, viewportY, viewportWidth, viewportHeight},
        focalLengthScale};
}

std::uint8_t LoadCombatGridColour(ZoneNumber zone)
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("GRID.DAT");
    fb.Seek((zone.mValue - 1) * 2);
    return static_cast<std::uint8_t>(fb.GetUint16LE());
}

}
