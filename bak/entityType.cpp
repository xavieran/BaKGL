#include "bak/entityType.hpp"

#include <utility>

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

ClipEffect GetClipEffect(EntityType entityType)
{
    using enum EntityType;
    switch (entityType)
    {
        case EXTERIOR:  [[fallthrough]];
        case BRIDGE:
            return ClipEffect::Follow;
        case INTERIOR:  [[fallthrough]];
        case HILL:      [[fallthrough]];
        case TERRAIN:   [[fallthrough]]; // unclear actually
        case BUILDING:
            return ClipEffect::Block;
        default:
            return ClipEffect::Ignore;
    }
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

glm::vec4 GetDebugColor(EntityType et)
{
    using enum EntityType;
    switch (et)
    {
    case TERRAIN:   return glm::vec4{0.50f, 0.50f, 0.50f, 0.4f};
    case EXTERIOR:  return glm::vec4{1.00f, 0.00f, 0.00f, 0.4f};
    case INTERIOR:  return glm::vec4{0.00f, 0.00f, 1.00f, 0.4f};
    case ENTRANCE:  return glm::vec4{1.00f, 1.00f, 0.00f, 0.4f};
    case BRIDGE:    return glm::vec4{1.00f, 0.00f, 1.00f, 0.4f};
    case BUILDING:  return glm::vec4{0.00f, 1.00f, 0.00f, 0.4f};
    case DOOR:      return glm::vec4{0.00f, 1.00f, 1.00f, 0.4f};
    default:
        return glm::vec4{0.60f, 0.60f, 0.60f, 0.4f};
    }

    std::unreachable();
}

std::string_view ToString(EntityType et)
{
    using enum EntityType;
    switch (et)
    {
        case TERRAIN:    return "TERRAIN";
        case EXTERIOR:   return "EXTERIOR";
        case BRIDGE:     return "BRIDGE";
        case INTERIOR:   return "INTERIOR";
        case HILL:       return "HILL";
        case TREE:       return "TREE";
        case CHEST:      return "CHEST";
        case DEADBODY1:  return "DEADBODY1";
        case FENCE:      return "FENCE";
        case GATE:       return "GATE";
        case BUILDING:   return "BUILDING";
        case TOMBSTONE:  return "TOMBSTONE";
        case SIGN:       return "SIGN";
        case TUNNEL1:    return "TUNNEL1";
        case PIT:        return "PIT";
        case DEADBODY2:  return "DEADBODY2";
        case DIRTPILE:   return "DIRTPILE";
        case CORN:       return "CORN";
        case FIRE:       return "FIRE";
        case ENTRANCE:   return "ENTRANCE";
        case GROVE:      return "GROVE";
        case FERN:       return "FERN";
        case DOOR:       return "DOOR";
        case CRYST:      return "CRYST";
        case ROCKPILE:   return "ROCKPILE";
        case BUSH1:      return "BUSH1";
        case BUSH2:      return "BUSH2";
        case BUSH3:      return "BUSH3";
        case SLAB:       return "SLAB";
        case STUMP:      return "STUMP";
        case WELL:       return "WELL";
        case ENGINE:     return "ENGINE";
        case SCARECROW:  return "SCARECROW";
        case TRAP:       return "TRAP";
        case CATAPULT:   return "CATAPULT";
        case COLUMN:     return "COLUMN";
        case LANDSCAPE:  return "LANDSCAPE";
        case TUNNEL2:    return "TUNNEL2";
        case BAG:        return "BAG";
        case LADDER:     return "LADDER";
        case DEAD_COMBATANT:  return "DEAD_COMBATANT";
        case LIVING_COMBATANT: return "LIVING_COMBATANT";
    }
    return "UNKNOWN";
}
}
