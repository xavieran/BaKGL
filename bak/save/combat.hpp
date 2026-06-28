#pragma once

#include "bak/combat/combat.hpp"
#include "bak/skills.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include <vector>

namespace BAK {

class Character;
class FileBuffer;
class Inventory;

std::vector<CombatEntityList> LoadCombatEntityLists(FileBuffer&);
std::vector<CombatantGridLocation> LoadCombatantGridLocations(FileBuffer&);
std::vector<CombatWorldLocation> LoadCombatWorldLocations(FileBuffer&);
std::vector<Skills> LoadCombatStats(FileBuffer&);
std::vector<Time> LoadCombatClickedTimes(FileBuffer&);

Character LoadCombatant(CombatantIndex, FileBuffer&, Inventory* inventory);

void Save(const std::vector<CombatantGridLocation>&, FileBuffer& fb);
void Save(const std::vector<CombatWorldLocation>&, FileBuffer&);
void Save(const std::vector<CombatEntityList>&, FileBuffer&);
void Save(const std::vector<Skills>&, FileBuffer&);
void Save(const std::vector<Character>&, FileBuffer&);
void Save(const std::vector<Time>&, FileBuffer&);

}
