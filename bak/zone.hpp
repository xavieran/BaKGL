#pragma once

#include "bak/encounter/encounter.hpp"
#include "bak/fixedObject.hpp"
#include "bak/resourceNames.hpp"
#include "bak/palette.hpp"
#include "bak/worldFactory.hpp"

#include "graphics/cube.hpp"
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
        mWorldTiles{mZoneItems, BAK::Encounter::EncounterFactory{}},
        mObjects{}
    {
        for (auto& item : mZoneItems.GetItems())
        mObjects.AddObject(
            item.GetName(),
            BAK::ZoneItemToMeshObject(item, mZoneTextures, mPalette));

        const auto cube = Graphics::Cuboid{1, 1, 50};
        mObjects.AddObject("Combat", cube.ToMeshObject(glm::vec4{1.0, 0, 0, .3}));
        mObjects.AddObject("Trap", cube.ToMeshObject(glm::vec4{.8, 0, 0, .3}));
        mObjects.AddObject("Dialog", cube.ToMeshObject(glm::vec4{0.0, 1, 0, .3}));
        //mObjects.AddObject("Dialog", cube.ToMeshObject(glm::vec4{0.0, 1, 0, .0}));
        mObjects.AddObject("Zone", cube.ToMeshObject(glm::vec4{1.0, 1, 0, .3}));
        mObjects.AddObject("GDSEntry", cube.ToMeshObject(glm::vec4{1.0, 0, 1, .3}));
        mObjects.AddObject("EventFlag", cube.ToMeshObject(glm::vec4{.0, .0, .7, .3}));
        mObjects.AddObject("Block", cube.ToMeshObject(glm::vec4{0,0,0, .3}));
    }

    
    ZoneLabel mZoneLabel;
    BAK::Palette mPalette;
    std::vector<FixedObject> mFixedObjects;
    BAK::ZoneTextureStore mZoneTextures;
    BAK::ZoneItemStore mZoneItems;
    BAK::WorldTileStore mWorldTiles;
    Graphics::MeshObjectStorage mObjects;
};

}
