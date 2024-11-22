#pragma once

#include <string>

namespace BAK {

constexpr auto DIALOG_POINTERS = "DEF_DIAL.DAT";
constexpr auto TOWN_DEFINITIONS = "DEF_TOWN.DAT";

class ZoneLabel
{
public:
    ZoneLabel(unsigned zoneNumber);
    ZoneLabel(const std::string& zoneLabel);

    std::string GetHorizon() const;
    std::string GetTerrain() const;
    std::string GetSpriteSlot(unsigned i) const;
    std::string GetPalette() const;
    std::string GetTile(unsigned x, unsigned y) const;
    std::string GetTileWorld(unsigned x, unsigned y) const;
    std::string GetTileData(unsigned x, unsigned y) const;
    std::string GetTable() const;
    std::string GetZone() const;
    std::string GetZoneLabel() const;
    unsigned GetZoneNumber() const;
    std::string GetZoneReference() const;
    std::string GetZoneDefault() const;
    std::string GetZoneMap() const;
    std::string GetZoneDat() const;
private:
    std::string mZoneLabel;
};

}
