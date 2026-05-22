#pragma once

#include "bak/combat.hpp"
#include "bak/skills.hpp"
#include "bak/spells.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include <vector>

namespace BAK {

class FileBuffer;

std::vector<CombatEntityList> LoadCombatEntityLists(FileBuffer&);
CombatEntityList LoadCombatEntityList(FileBuffer&, CombatIndex);
std::vector<CombatantGridLocation> LoadCombatantGridLocations(FileBuffer&);
CombatantGridLocation LoadCombatantGridLocation(FileBuffer&, CombatIndex);
std::vector<CombatWorldLocation> LoadCombatWorldLocations(FileBuffer&);
std::vector<Skills> LoadCombatStats(FileBuffer&);
std::vector<Time> LoadCombatClickedTimes(FileBuffer&);

}
