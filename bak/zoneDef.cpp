#include "bak/zoneDef.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/resourceNames.hpp"

#include "com/logger.hpp"

namespace BAK {


ZoneDefaults LoadZoneDefDat(ZoneNumber zone)
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

    Logging::LogDebug(__FUNCTION__) << "ZoneType: " << zoneType << " 3dParam? " << threeDParam <<"\n";
    Logging::LogDebug(__FUNCTION__) << "PlayerPos A: " << playerPos_fieldA << " PlayerPos E: " << playerPos_fieldE << "\n";
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
        static_cast<float>(mapZoomRate)};
}

};
