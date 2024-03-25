#pragma once

#include <functional>
#include <sstream>
#include <iomanip>

namespace BAK {

constexpr auto DIALOG_POINTERS = "DEF_DIAL.DAT";
constexpr auto TOWN_DEFINITIONS = "DEF_TOWN.DAT";

class ZoneLabel
{
public:
    ZoneLabel(unsigned zoneNumber)
    :
        mZoneLabel{std::invoke([&]{
            std::stringstream ss{};
            ss << "Z" << std::setw(2) << std::setfill('0') << zoneNumber;
            return ss.str();
        })}
    {}

    ZoneLabel(const std::string& zoneLabel)
    :
        mZoneLabel{zoneLabel}
    {}

    std::string GetHorizon() const
    {
        std::stringstream ss{""};
        ss << GetZone() << "H.SCX";
        return ss.str();
    }

    std::string GetTerrain() const
    {
        std::stringstream ss{""};
        ss << GetZone() << "L.SCX";
        return ss.str();
    }

    std::string GetSpriteSlot(unsigned i) const
    {
        std::stringstream ss{""};
        ss << GetZone() << "SLOT" << std::setfill('0') 
            << std::setw(1) << i << ".BMX";
        return ss.str();
    }

    std::string GetPalette() const
    {
        std::stringstream ss{""};
        ss << GetZone() << ".PAL";
        return ss.str();
    }

    std::string GetTile(unsigned x, unsigned y) const
    {
        std::stringstream ss{""};
        ss << "T" << std::setfill('0') << mZoneLabel.substr(1,2)
            << std::setw(2) << x << std::setw(2) << y;
        return ss.str();
    }

    std::string GetTileWorld(unsigned x, unsigned y) const
    {
        return GetTile(x, y) + ".WLD";
    }

    std::string GetTileData(unsigned x, unsigned y) const
    {
        return GetTile(x, y) + ".DAT";
    }

    std::string GetTable() const
    {
        std::stringstream ss{""};
        ss << GetZoneLabel() << ".TBL";
        return ss.str();
    }

    std::string GetZone() const
    {
        return mZoneLabel.substr(0, 3);
    }

    std::string GetZoneLabel() const
    {
        return mZoneLabel;
    }

    unsigned GetZoneNumber() const
    {
        return std::atoi(mZoneLabel.substr(1,2).c_str());
    }

    std::string GetZoneReference() const
    {
        std::stringstream ss{""};
        ss << GetZoneLabel() << "REF.DAT";
        return ss.str();
    };

    std::string GetZoneDefault() const
    {
        std::stringstream ss{""};
        ss << GetZoneLabel() << "DEF.DAT";
        return ss.str();
    };

    std::string GetZoneMap() const
    {
        std::stringstream ss{""};
        ss << GetZoneLabel() << "MAP.DAT";
        return ss.str();
    };
private:
    std::string mZoneLabel;
};

}
