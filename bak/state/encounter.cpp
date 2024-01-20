#include "bak/state/encounter.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "com/logger.hpp"

namespace BAK::State {

// Called by checkBlockTriggered, checkTownTriggered, checkBackgroundTriggered, checkZoneTriggered,
// doEnableEncounter, doDialogEncounter, doDisableEncounter, doSoundEncounter
bool CheckActive(
    FileBuffer& fb,
    const Encounter::Encounter& encounter,
    ZoneNumber zone) 
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    const bool alreadyEncountered = CheckUniqueEncounterStateFlagOffset(
        fb,
        zone,
        encounter.GetTileIndex(),
        encounterIndex);
    const bool recentlyEncountered = CheckRecentlyEncountered(fb, encounterIndex);
    // event flag 1 - this flag must be set to encounter the event
    const bool eventFlag1 = encounter.mSaveAddress != 0
        ? (ReadEventBool(fb, encounter.mSaveAddress) == 1)
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mSaveAddress2 != 0
        ? ReadEventBool(fb, encounter.mSaveAddress2)
        : false;
    return !(alreadyEncountered
        || recentlyEncountered
        || eventFlag1
        || eventFlag2);
}

bool CheckCombatActive(
    FileBuffer& fb,
    const Encounter::Encounter& encounter,
    ZoneNumber zone) 
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    const bool alreadyEncountered = CheckUniqueEncounterStateFlagOffset(
        fb,
        zone,
        encounter.GetTileIndex(),
        encounterIndex);

    assert(std::holds_alternative<Encounter::Combat>(encounter.GetEncounter()));
    const auto combatIndex = std::get<Encounter::Combat>(encounter.GetEncounter()).mCombatIndex;

    // If this flag is set then this combat hasn't been seen
    const bool encounterFlag1464 = CheckCombatEncounterStateFlag(fb, combatIndex);

    // event flag 1 - this flag must be set to encounter the event
    const bool eventFlag1 = encounter.mSaveAddress != 0
        ? (ReadEventBool(fb, encounter.mSaveAddress) == 1)
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mSaveAddress2 != 0
        ? ReadEventBool(fb, encounter.mSaveAddress2)
        : false;

    Logging::LogInfo(__FUNCTION__) << " alreadyEncountered: " << alreadyEncountered
        << " combatEncounterStateFlag: " << encounterFlag1464
        << " eventFlag1: " << eventFlag1
        << " eventFlag2: " << eventFlag2 << "\n";

    return !(alreadyEncountered
        || encounterFlag1464
        || eventFlag1
        || eventFlag2);
}

void SetPostDialogEventFlags(
    FileBuffer& fb,
    const Encounter::Encounter& encounter,
    ZoneNumber zone)
{
    const auto tileIndex = encounter.GetTileIndex();
    const auto encounterIndex = encounter.GetIndex().mValue;

    if (encounter.mSaveAddress3 != 0)
    {
        SetEventFlagTrue(fb, encounter.mSaveAddress3);
    }

    // Unknown 3 flag is associated with events like the sleeping glade and 
    // timirianya danger zone (effectively, always encounter this encounter)
    if (encounter.mUnknown3 == 0)
    {
        // Inhibit for this chapter
        if (encounter.mUnknown2 != 0)
        {
            SetEventFlagTrue(
                fb,
                CalculateUniqueEncounterStateFlagOffset(
                    zone,
                    tileIndex,
                    encounterIndex));
        }

        // Inhibit for this tile
        SetRecentlyEncountered(fb, encounterIndex);
    }

}

// Background and Town
void SetPostGDSEventFlags(
    FileBuffer& fb,
    const Encounter::Encounter& encounter)
{
    if (encounter.mSaveAddress3 != 0)
        SetEventFlagTrue(fb, encounter.mSaveAddress3);
}

// Used by Block, Disable, Enable, Sound, Zone
void SetPostEnableOrDisableEventFlags(
    FileBuffer& fb,
    const Encounter::Encounter& encounter,
    ZoneNumber zone)
{
    if (encounter.mSaveAddress3 != 0)
    {
        SetEventFlagTrue(fb, encounter.mSaveAddress3);
    }

    if (encounter.mUnknown2 != 0)
    {
        SetEventFlagTrue(
            fb,
            CalculateUniqueEncounterStateFlagOffset(
                zone,
                encounter.GetTileIndex(),
                encounter.GetIndex().mValue));
    }
}


// For each encounter in every zone there is a unique enabled/disabled flag.
// This is reset every time a new chapter is loaded (I think);
unsigned CalculateUniqueEncounterStateFlagOffset(
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex) 
{
    constexpr auto encounterStateOffset = 0x190;
    constexpr auto maxEncountersPerTile = 0xa;
    const auto zoneOffset = (zone.mValue - 1) * encounterStateOffset;
    const auto tileOffset = tileIndex * maxEncountersPerTile;
    const auto offset = zoneOffset + tileOffset + encounterIndex;
    return offset + encounterStateOffset;
}

bool CheckUniqueEncounterStateFlagOffset(
    FileBuffer& fb,
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex) 
{
    return ReadEventBool(
        fb,
        CalculateUniqueEncounterStateFlagOffset(
            zone,
            tileIndex,
            encounterIndex));
}

unsigned CalculateCombatEncounterScoutedStateFlag(
    std::uint8_t encounterIndex) 
{
    constexpr auto offset = 0x145a;
    return offset + encounterIndex;
}

unsigned CalculateCombatEncounterStateFlag(
    unsigned combatIndex) 
{
    constexpr auto combatEncounterFlag = 0x1464;
    return combatIndex + combatEncounterFlag;
}

bool CheckCombatEncounterStateFlag(
    FileBuffer& fb,
    unsigned combatIndex) 
{
    constexpr auto alwaysTriggeredIndex = 0x3e8;
    if (combatIndex >= alwaysTriggeredIndex)
        return true;
    else
        return ReadEventBool(
            fb,
            CalculateCombatEncounterStateFlag(combatIndex));
}

void SetCombatEncounterScoutedState(
    FileBuffer& fb,
    std::uint8_t encounterIndex,
    bool state)
{
    SetEventFlag(fb, CalculateCombatEncounterScoutedStateFlag(encounterIndex), state);
}

// 1450 is "recently encountered this encounter"
// should be cleared when we move to a new tile
// (or it will inhibit the events of the new tile)
unsigned CalculateRecentEncounterStateFlag(
    std::uint8_t encounterIndex) 
{
    // Refer readEncounterEventState1450 in IDA
    // These get cleared when we load a new tile
    constexpr auto offset = 0x1450;
    return offset + encounterIndex;
}

bool CheckRecentlyEncountered(FileBuffer& fb, std::uint8_t encounterIndex)
{
    return ReadEventBool(fb, CalculateRecentEncounterStateFlag(encounterIndex));
}

void SetRecentlyEncountered(FileBuffer& fb, std::uint8_t encounterIndex)
{
    SetEventFlagTrue(
        fb,
        CalculateRecentEncounterStateFlag(
            encounterIndex));
}

void ClearTileRecentEncounters(
    FileBuffer& fb)
{
    for (unsigned i = 0; i < 10; i++)
    {
        SetEventFlagFalse(fb, CalculateRecentEncounterStateFlag(i));
        SetEventFlagFalse(fb, CalculateCombatEncounterScoutedStateFlag(i));
    }
}
}
