#pragma once

#include "bak/types.hpp"

namespace BAK {

class Inventory;
class Party;
class Character;
class FileBuffer;
class GenericContainer;
class MapLocation;
class Location;
class SpellState;
class TimeExpiringState;
class WorldClock;

void Save(const Inventory&, FileBuffer&);
void Save(const GenericContainer&, FileBuffer&);
void Save(const Character&, FileBuffer&);
void Save(const Party&, FileBuffer&);
void Save(const WorldClock&, FileBuffer&);
void Save(const std::vector<TimeExpiringState>& storage, FileBuffer& fb);
void Save(const SpellState& spells, FileBuffer& fb);
void Save(Chapter chapter, FileBuffer& fb);
void Save(const MapLocation& location, FileBuffer& fb);
void Save(const Location& location, FileBuffer& fb);

}
