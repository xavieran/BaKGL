#pragma once

#include "bak/types.hpp"

namespace BAK {
class FileBuffer;
class GameState;
struct Time;
namespace Encounter {
class Encounter;
}
}

namespace BAK::State {

// Called by
// * checkBlockTriggered
// * checkTownTriggered
// * checkBackgroundTriggered
// * checkZoneTriggered,
// * doEnableEncounter
// * doDialogEncounter
// * doDisableEncounter
// * doSoundEncounter
bool CheckEncounterActive(
    const GameState&,
    const Encounter::Encounter& encounter,
    ZoneNumber zone);

bool CheckCombatActive(
    const GameState&,
    const Encounter::Encounter& encounter,
    ZoneNumber zone);

// Used by
// * Dialog
void SetPostDialogEventFlags(
    FileBuffer&,
    const Encounter::Encounter& encounter,
    ZoneNumber zone);

// Used by
// * Background
// * Town
void SetPostGDSEventFlags(
    FileBuffer&,
    const Encounter::Encounter& encounter);

// Used by
// * Block
// * Disable
// * Enable
// * Sound
// * Zone
void SetPostEnableOrDisableEventFlags(
    FileBuffer&,
    const Encounter::Encounter& encounter,
    ZoneNumber zone);

// For each encounter in every zone there is a unique enabled/disabled flag.
// This is reset every time a new chapter is loaded (I think);
unsigned CalculateUniqueEncounterStateFlagOffset(
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex);

bool CheckUniqueEncounterStateFlag(
    const GameState&,
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex);

void SetUniqueEncounterStateFlag(
    FileBuffer& fb,
    ZoneNumber zone,
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex,
    bool value);

// 1450 is "recently encountered this encounter"
// should be cleared when we move to a new tile
// (or it will inhibit the events of the new tile)
unsigned CalculateRecentEncounterStateFlag(
    std::uint8_t encounterIndex);

// 1464 is combat completed flag
unsigned CalculateCombatEncounterStateFlag(
    unsigned combatIndex);

bool CheckCombatEncounterStateFlag(
    const GameState&,
    unsigned combatIndex);

void SetCombatEncounterState(
    FileBuffer& fb,
    unsigned combatIndex,
    bool state);

// 145a is combat scouted flag
unsigned CalculateCombatEncounterScoutedStateFlag(
    std::uint8_t encounterIndex);

void SetCombatEncounterScoutedState(
    FileBuffer&,
    std::uint8_t encounterIndex, bool state);

void SetRecentlyEncountered(FileBuffer&, std::uint8_t encounterIndex);
bool CheckRecentlyEncountered(const GameState&, std::uint8_t encounterIndex);
void ClearTileRecentEncounters(FileBuffer&);
void SetPostCombatCombatSpecificFlags(GameState& gs, unsigned combatIndex);

Time GetCombatClickedTime(FileBuffer& fb, unsigned combatIndex);
void SetCombatClickedTime(FileBuffer& fb, unsigned combatIndex, Time time);

}
