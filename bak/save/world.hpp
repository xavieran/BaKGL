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
bool LoadFollowRoad(FileBuffer&);
void LoadChapterOffsetP(FileBuffer&);

void Save(const WorldClock&, FileBuffer&);
void Save(const std::vector<TimeExpiringState>&, FileBuffer&);
void Save(const SpellState&, FileBuffer&);
void Save(bool followRoad, FileBuffer&);
void Save(Chapter, FileBuffer&);
void Save(const MapLocation&, FileBuffer&);
void Save(const Location&, FileBuffer&);

}
