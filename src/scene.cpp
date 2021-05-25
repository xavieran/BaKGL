#include "src/scene.hpp"

#include "src/glm.hpp"

namespace BAK {

std::string_view ToString(HotspotAction ha)
{
    switch (ha)
    {
        case HotspotAction::DIALOG:    return "Dialog";
        case HotspotAction::EXIT:      return "Exit";
        case HotspotAction::GOTO:      return "Goto";
        case HotspotAction::BARMAID:   return "Barmaid";
        case HotspotAction::SHOP:      return "Shop";
        case HotspotAction::INN:       return "Inn";
        case HotspotAction::CONTAINER: return "Container";
        case HotspotAction::LUTE:      return "Lute";
        case HotspotAction::TELEPORT:  return "Teleport";
        case HotspotAction::TEMPLE:    return "Temple";
        case HotspotAction::NOT_SURE:  return "Not Sure";
        case HotspotAction::REPAIR:    return "Repair";

        default: throw std::runtime_error("Unknown HotspotAction");
    }
}

std::ostream& operator<<(std::ostream& os, HotspotAction ha)
{
    return os << ToString(ha);
}

std::ostream& operator<<(std::ostream& os, const Hotspot& hs)
{
    os << "Hotspot { id: " << hs.mHotspot << " topLeft: " << hs.mTopLeft
        << " dim: " << hs.mDimensions << " kw: " << hs.mKeyword 
        << " act: " << hs.mAction << " tip: " << std::hex << hs.mTooltip
        << " dialog: " << hs.mDialog << std::dec << "}";
    return os;
}

}
