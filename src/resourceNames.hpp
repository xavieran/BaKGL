#pragma once

#include <sstream>

namespace BAK {

class ZoneLabel
{
public:

    ZoneLabel(const std::string& zoneLabel)
    :
        mZoneLabel{zoneLabel}
    {}

    std::string GetDialogPointers() const
    {
        return "DEF_DIAL.DAT";
    }

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

private:
    const std::string mZoneLabel;
};

}
