#include "bak/entityType.hpp"

namespace BAK {

unsigned GetContainerTypeFromEntityType(EntityType et)
{
    using enum EntityType;
    switch (et)
    {
        case BAG: return 0;
        case CHEST: return 1;
        case DIRTPILE: return 2;
        case TOMBSTONE: return 3;
        case DEADBODY1: [[fallthrough]];
        case DEADBODY2: return 4;
        case TRAP: return 5;
        case BUSH1: [[fallthrough]];
        case BUSH2: [[fallthrough]];
        case BUSH3: return 6;
        // 7 is shop, which is not an entity
        case CRYST: return 8;
        case STUMP: return 9;
        case BUILDING: return 10;
        default: return 0;
    }
}

EntityType EntityTypeFromModelName(std::string_view name)
{
    if (name.substr(0, 3) == "box") return EntityType::CHEST;
    if (name.substr(0, 5) == "chest") return EntityType::CHEST;
    if (name.substr(0, 4) == "gate") return EntityType::GATE;
    if (name.substr(0, 5) == "stump") return EntityType::STUMP;
    return EntityType::CHEST;
}

GridEffect GetGridEffect(EntityType entityType)
{
    using enum EntityType;
    switch (entityType)
    {
        // TODO: Check these - not sure on all of them
        case TERRAIN:   [[fallthrough]];
        case EXTERIOR:  [[fallthrough]];
        case BRIDGE:    [[fallthrough]];
        case INTERIOR:
            return GridEffect::None;
        case TREE:      [[fallthrough]];
        case FENCE:     [[fallthrough]];
        case GROVE:     [[fallthrough]];
        case FERN:      [[fallthrough]];
        case CORN:      [[fallthrough]];
        case COLUMN:    [[fallthrough]];
        case SCARECROW: [[fallthrough]];
        case SLAB:      [[fallthrough]];
        case ROCKPILE:  [[fallthrough]];
        case DEADBODY1: [[fallthrough]];
        case DEADBODY2:
            return GridEffect::Hidden;
        default:
            return GridEffect::NotWalkable;
    }
}
}
