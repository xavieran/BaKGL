#pragma once

#include "bak/fixedObject.hpp"
#include "bak/resourceNames.hpp"
#include "bak/palette.hpp"
#include "bak/worldFactory.hpp"

#include "graphics/meshObject.hpp"

namespace BAK {

// Contains all the data one would need for a zone
class Zone
{
public:

    Zone(unsigned zoneNumber)
    :
        mZoneLabel{zoneNumber},
        mPalette{mZoneLabel.GetPalette()},
        mFixedObjects{LoadFixedObjects(zoneNumber)},
        mZoneTextures{mZoneLabel, mPalette},
        mZoneItems{mZoneLabel, mZoneTextures},
        mObjects{}
    {}
    
    ZoneLabel mZoneLabel;
    BAK::Palette mPalette;
    std::vector<FixedObject> mFixedObjects;
    BAK::ZoneTextureStore mZoneTextures;
    BAK::ZoneItemStore mZoneItems;
    Graphics::MeshObjectStorage mObjects;
};

}
