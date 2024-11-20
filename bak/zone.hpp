#pragma once

#include "bak/container.hpp"
#include "bak/palette.hpp"
#include "bak/resourceNames.hpp"
#include "bak/worldFactory.hpp"

#include "graphics/meshObject.hpp"

#include <vector>

namespace BAK {

// Contains all the data one would need for a zone
class Zone
{
public:
    Zone(unsigned zoneNumber);

    ZoneLabel mZoneLabel;
    Palette mPalette;
    std::vector<GenericContainer> mFixedObjects;
    ZoneTextureStore mZoneTextures;
    ZoneItemStore mZoneItems;
    WorldTileStore mWorldTiles;
    Graphics::MeshObjectStorage mObjects;
};

}
