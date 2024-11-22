#include "bak/resourceNames.hpp"

#include <functional>
#include <sstream>
#include <iomanip>

namespace BAK {

ZoneLabel::ZoneLabel(unsigned zoneNumber)
:
    mZoneLabel{std::invoke([&]{
        std::stringstream ss{};
        ss << "Z" << std::setw(2) << std::setfill('0') << zoneNumber;
        return ss.str();
    })}
{}

ZoneLabel::ZoneLabel(const std::string& zoneLabel)
:
    mZoneLabel{zoneLabel}
{}

std::string ZoneLabel::GetHorizon() const
{
    std::stringstream ss{""};
    ss << GetZone() << "H.SCX";
    return ss.str();
}

std::string ZoneLabel::GetTerrain() const
{
    std::stringstream ss{""};
    ss << GetZone() << "L.SCX";
    return ss.str();
}

std::string ZoneLabel::GetSpriteSlot(unsigned i) const
{
    std::stringstream ss{""};
    ss << GetZone() << "SLOT" << std::setfill('0') 
        << std::setw(1) << i << ".BMX";
    return ss.str();
}

std::string ZoneLabel::GetPalette() const
{
    std::stringstream ss{""};
    ss << GetZone() << ".PAL";
    return ss.str();
}

std::string ZoneLabel::GetTile(unsigned x, unsigned y) const
{
    std::stringstream ss{""};
    ss << "T" << std::setfill('0') << mZoneLabel.substr(1,2)
        << std::setw(2) << x << std::setw(2) << y;
    return ss.str();
}

std::string ZoneLabel::GetTileWorld(unsigned x, unsigned y) const
{
    return GetTile(x, y) + ".WLD";
}

std::string ZoneLabel::GetTileData(unsigned x, unsigned y) const
{
    return GetTile(x, y) + ".DAT";
}

std::string ZoneLabel::GetTable() const
{
    std::stringstream ss{""};
    ss << GetZoneLabel() << ".TBL";
    return ss.str();
}

std::string ZoneLabel::GetZone() const
{
    return mZoneLabel.substr(0, 3);
}

std::string ZoneLabel::GetZoneLabel() const
{
    return mZoneLabel;
}

unsigned ZoneLabel::GetZoneNumber() const
{
    return std::atoi(mZoneLabel.substr(1,2).c_str());
}

std::string ZoneLabel::GetZoneReference() const
{
    std::stringstream ss{""};
    ss << GetZoneLabel() << "REF.DAT";
    return ss.str();
};

std::string ZoneLabel::GetZoneDefault() const
{
    std::stringstream ss{""};
    ss << GetZoneLabel() << "DEF.DAT";
    return ss.str();
};

std::string ZoneLabel::GetZoneMap() const
{
    std::stringstream ss{""};
    ss << GetZoneLabel() << "MAP.DAT";
    return ss.str();
};

std::string ZoneLabel::GetZoneDat() const
{
    std::stringstream ss{""};
    ss << GetZoneLabel() << ".DAT";
    return ss.str();
};

}
