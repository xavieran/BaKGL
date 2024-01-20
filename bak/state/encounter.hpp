#pragma once

#include "bak/file/fileBuffer.hpp"

#include "bak/encounter/encounter.hpp"

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
bool CheckActive(
    FileBuffer&,
    const Encounter::Encounter& encounter,
    ZoneNumber zone);

bool CheckCombatActive(
    FileBuffer&,
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

bool CheckUniqueEncounterStateFlagOffset(
    FileBuffer&,
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex);

// 1450 is "recently encountered this encounter"
// should be cleared when we move to a new tile
// (or it will inhibit the events of the new tile)
unsigned CalculateRecentEncounterStateFlag(
    std::uint8_t encounterIndex);

// 145a is combat scouted flag
unsigned CalculateCombatEncounterStateFlag(
    unsigned combatIndex);

// 1464 is combat completed flag
unsigned CalculateCombatEncounterScoutedStateFlag(
    std::uint8_t encounterIndex);

bool CheckCombatEncounterStateFlag(
    FileBuffer&,
    unsigned combatIndex);

void SetCombatEncounterScoutedState(
    FileBuffer&,
    std::uint8_t encounterIndex, bool state);

void SetRecentlyEncountered(FileBuffer&, std::uint8_t encounterIndex);
bool CheckRecentlyEncountered(FileBuffer&, std::uint8_t encounterIndex);
void ClearTileRecentEncounters(FileBuffer&);

}
