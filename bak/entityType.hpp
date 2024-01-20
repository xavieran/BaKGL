#pragma once

namespace BAK {

enum class EntityType
{
    TERRAIN    =  0,
    EXTERIOR   =  1,
    BRIDGE     =  2,
    INTERIOR   =  3,
    HILL       =  4,
    TREE       =  5,
    CHEST      =  6,
    DEADBODY1  =  7,
    FENCE      =  8,
    GATE       =  9, // RIFT GATE
    BUILDING   = 10,
    TOMBSTONE  = 12,
    SIGN       = 13,
    TUNNEL1    = 14, // ALSO TUNNEL...
    PIT        = 15,
    DEADBODY2  = 16,
    DIRTPILE   = 17,
    CORN       = 18,
    FIRE       = 19,
    ENTRANCE   = 20,
    GROVE      = 21,
    FERN       = 22,
    DOOR       = 23,
    CRYST      = 24,
    ROCKPILE   = 25,
    BUSH1      = 26,
    BUSH2      = 27,
    BUSH3      = 28,
    SLAB       = 29,
    STUMP      = 30,
    WELL       = 31,
    ENGINE     = 33,
    SCARECROW  = 34,
    TRAP       = 35,
    CATAPULT   = 36,
    COLUMN     = 37,
    LANDSCAPE  = 38,
    TUNNEL2    = 39, // with tunnel
    BAG        = 41,
    LADDER     = 42
};

unsigned GetContainerTypeFromEntityType(EntityType);

}
