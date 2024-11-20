#pragma once

#include <vector>

namespace Graphics {
class MeshObjectStorage;
}

namespace BAK {

class GenericContainer;
class Palette;
class ZoneLabel;
class ZoneTextureStore;
class ZoneItemStore;
class WorldTileStore;

// Contains all the data one would need for a zone
class Zone
{
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
