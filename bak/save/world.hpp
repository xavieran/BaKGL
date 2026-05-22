#pragma once

#include "bak/types.hpp"
#include "bak/worldClock.hpp"
#include "bak/coordinates.hpp"
#include "bak/spells.hpp"
#include "bak/timeExpiringState.hpp"

#include <vector>

namespace BAK {

class FileBuffer;

unsigned LoadChapter(FileBuffer&);
WorldClock LoadWorldTime(FileBuffer&);
MapLocation LoadMapLocation(FileBuffer&);
Location LoadLocation(FileBuffer&);
std::vector<TimeExpiringState> LoadTimeExpiringState(FileBuffer&);
SpellState LoadSpells(FileBuffer&);
void LoadChapterOffsetP(FileBuffer&);

}
