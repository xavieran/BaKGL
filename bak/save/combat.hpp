#pragma once

#include "bak/combat/combat.hpp"
#include "bak/skills.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include <vector>

namespace BAK {

class FileBuffer;

std::vector<CombatEntityList> LoadCombatEntityLists(FileBuffer&);
std::vector<CombatantGridLocation> LoadCombatantGridLocations(FileBuffer&);
std::vector<CombatWorldLocation> LoadCombatWorldLocations(FileBuffer&);
std::vector<Skills> LoadCombatStats(FileBuffer&);
std::vector<Time> LoadCombatClickedTimes(FileBuffer&);

void Save(const std::vector<CombatantGridLocation>&, FileBuffer& fb);
void Save(const std::vector<CombatWorldLocation>&, FileBuffer&);
void Save(const std::vector<CombatEntityList>&, FileBuffer&);

}
