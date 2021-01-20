#pragma once

#include "FileBuffer.h"

#include "TileWorldResource.h"
#include "TableResource.h"

#include <sstream>
#include <iomanip>   
#include <iostream>   
#include <cassert>   

namespace BAK {

enum class EntityType : unsigned
{
    ET_TERRAIN    =  0,
    ET_EXTERIOR   =  1,
    ET_BRIDGE     =  2,
    ET_INTERIOR   =  3,
    ET_HILL       =  4,
    ET_TREE       =  5,
    ET_CHEST      =  6,
    ET_DEADBODY1  =  7,
    ET_FENCE      =  8,
    ET_GATE       =  9,
    ET_BUILDING   = 10,
    ET_TOMBSTONE  = 12,
    ET_SIGN       = 13,
    ET_ROOM       = 14,
    ET_PIT        = 15,
    ET_DEADBODY2  = 16,
    ET_DIRTPILE   = 17,
    ET_CORN       = 18,
    ET_FIRE       = 19,
    ET_ENTRANCE   = 20,
    ET_GROVE      = 21,
    ET_FERN       = 22,
    ET_DOOR       = 23,
    ET_CRYST      = 24,
    ET_ROCKPILE   = 25,
    ET_BUSH1      = 26,
    ET_BUSH2      = 27,
    ET_BUSH3      = 28,
    ET_SLAB       = 29,
    ET_STUMP      = 30,
    ET_WELL       = 31,
    ET_ENGINE     = 33,
    ET_SCARECROW  = 34,
    ET_TRAP       = 35,
    ET_CATAPULT   = 36,
    ET_COLUMN     = 37,
    ET_LANDSCAPE  = 38,
    ET_MOUNTAIN   = 39,
    ET_BAG        = 41,
    ET_LADDER     = 42
};

enum class TerrainType : unsigned
{
    TT_NULL   = 0,
    TT_RIVER  = 6,
    TT_ROAD   = 7,
    TT_GROUND = 8
};

enum class EntityProperty : unsigned
{
    Scenery = 0,
    Container = 1 << 1,
    Popup = 1 << 2
};

}
