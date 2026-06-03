#include "bak/state/encounter.hpp"

#include "bak/encounter/encounter.hpp"
#include "bak/file/fileBuffer.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

namespace BAK::State {

// Called by checkBlockTriggered, checkTownTriggered, checkBackgroundTriggered, checkZoneTriggered,
// doEnableEncounter, doDialogEncounter, doDisableEncounter, doSoundEncounter
bool CheckEncounterActive(
    const GameState& gs,
    const Encounter::Encounter& encounter,
    ZoneNumber zone) 
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    const bool alreadyEncountered = CheckUniqueEncounterStateFlag(
        gs,
        zone,
        encounter.GetTileIndex(),
        encounterIndex);
    const bool recentlyEncountered = CheckRecentlyEncountered(gs, encounterIndex);
    // event flag 1 - this flag must be set to encounter the event
    const bool eventFlag1 = encounter.mRequiredState != 0
        ? !gs.GetEventState(CreateChoice(encounter.mRequiredState))
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mInhibitState != 0
        ? gs.GetEventState(CreateChoice(encounter.mInhibitState))
        : false;
    return !(alreadyEncountered
        || recentlyEncountered
        || eventFlag1
        || eventFlag2);
}

bool CheckCombatActive(
    const GameState& gs,
    const Encounter::Encounter& encounter,
    ZoneNumber zone) 
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    const bool alreadyEncountered = CheckUniqueEncounterStateFlag(
        gs,
        zone,
        encounter.GetTileIndex(),
        encounterIndex);

    assert(std::holds_alternative<Encounter::Combat>(encounter.GetEncounter()));
    const auto combatIndex = std::get<Encounter::Combat>(encounter.GetEncounter()).mCombatIndex;

    // If this flag is set then this combat hasn't been seen
    const bool encounterFlag1464 = CheckCombatEncounterStateFlag(gs, combatIndex);

    // event flag 1 - this flag must be set to encounter the event
    const bool eventFlag1 = encounter.mRequiredState != 0
        ? !gs.GetEventState(CreateChoice(encounter.mRequiredState))
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mInhibitState != 0
        ? gs.GetEventState(CreateChoice(encounter.mInhibitState))
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

    if (encounter.mCompletionState != 0)
    {
        SetEventFlagTrue(fb, encounter.mCompletionState);
    }

    // Repeatable flag is associated with events like the sleeping glade and 
    // timirianya danger zone (effectively, always encounter this encounter)
    if (encounter.mRepeatable == 0)
    {
        // Inhibit for this chapter
        if (encounter.mChapterFlag != 0)
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
    if (encounter.mCompletionState != 0)
        SetEventFlagTrue(fb, encounter.mCompletionState);
}

// Used by Block, Disable, Enable, Sound, Zone
void SetPostEnableOrDisableEventFlags(
    FileBuffer& fb,
    const Encounter::Encounter& encounter,
    ZoneNumber zone)
{
    if (encounter.mCompletionState != 0)
    {
        SetEventFlagTrue(fb, encounter.mCompletionState);
    }

    if (encounter.mChapterFlag != 0)
    {
        SetUniqueEncounterStateFlag(
            fb,
            zone,
            encounter.GetTileIndex(),
            encounter.GetIndex().mValue,
            true);
    }
}


// For each encounter in every zone there is a unique enabled/disabled flag.
// This is reset every time a new chapter is loaded (I think);
unsigned CalculateUniqueEncounterStateFlagOffset(
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex) 
{
    constexpr auto maxEncountersPerTile = 0xa;
    const auto zoneOffset = (zone.mValue - 1) * sEncounterStateOffset;
    const auto tileOffset = tileIndex * maxEncountersPerTile;
    const auto offset = zoneOffset + tileOffset + encounterIndex;
    return offset + sEncounterStateOffset;
}

bool CheckUniqueEncounterStateFlag(
    const GameState& gs,
    ZoneNumber zone, 
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex) 
{
    return gs.ReadEventBool(
        CalculateUniqueEncounterStateFlagOffset(
            zone,
            tileIndex,
            encounterIndex));
}

void SetUniqueEncounterStateFlag(
    FileBuffer& fb,
    ZoneNumber zone,
    std::uint8_t tileIndex,
    std::uint8_t encounterIndex,
    bool value)
{
    SetEventFlag(
        fb,
        CalculateUniqueEncounterStateFlagOffset(
            zone,
            tileIndex,
            encounterIndex),
        value);
}

unsigned CalculateCombatEncounterScoutedStateFlag(
    std::uint8_t encounterIndex) 
{
    return sCombatScoutedOffset + encounterIndex;
}

unsigned CalculateCombatEncounterStateFlag(
    CombatIndex combatIndex) 
{
    return combatIndex.mValue + sCombatEncounterOffset;
}

bool CheckCombatEncounterStateFlag(
    const GameState& gs,
    CombatIndex index)
{
    if (index.mValue >= sMaxCombatIndex)
        return true;
    else
        return gs.ReadEventBool(
            CalculateCombatEncounterStateFlag(index));
}

void SetCombatEncounterState(
    FileBuffer& fb,
    CombatIndex combatIndex,
    bool state)
{
    if (combatIndex.mValue >= sMaxCombatIndex)
    {
        return;
    }
    SetEventFlag(fb, CalculateCombatEncounterStateFlag(combatIndex), state);
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
    return sRecentEncounterOffset + encounterIndex;
}

bool CheckRecentlyEncountered(const GameState& gs, std::uint8_t encounterIndex)
{
    return gs.ReadEventBool(CalculateRecentEncounterStateFlag(encounterIndex));
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

void SetPostCombatCombatSpecificFlags(
    GameState& gs,
    const Encounter::Encounter& encounter,
    CombatIndex combatIndex)
{
    switch (combatIndex.mValue)
    {
        case 74: return; // this runs dialog 1cfdf1 (nago combat flags)
                         // and is handled externally in my code
        // These are the Eortis quest rusalki combats
        case 131: [[fallthrough]];
        case 132: [[fallthrough]];
        case 133: [[fallthrough]];
        case 134: [[fallthrough]];
        case 135:
            if (gs.ReadEventBool(0x14e7)
                && gs.ReadEventBool(0x14e8)
                && gs.ReadEventBool(0x14e9)
                && gs.ReadEventBool(0x14ea)
                && gs.ReadEventBool(0x14eb))
            {
                gs.SetEventValue(0xdb1c, 1);
            }
            return;
        // Never-ending combats
        case 235: [[fallthrough]];
        case 245: [[fallthrough]];
        case 291: [[fallthrough]];
        case 293: [[fallthrough]];
        case 335: [[fallthrough]];
        case 337: [[fallthrough]];
        case 338: [[fallthrough]];
        case 375: [[fallthrough]];
        case 410: [[fallthrough]];
        case 429: [[fallthrough]];
        case 430:
            gs.ReactivateCombat(encounter, BAK::CombatIndex{combatIndex});
            break;
        case 610: [[fallthrough]];
        case 613: [[fallthrough]];
        case 615: [[fallthrough]];
        case 618: [[fallthrough]];
        case 619: [[fallthrough]];
        case 621:
            if (gs.ReadEventBool(0x16c6)
                && gs.ReadEventBool(0x16c9)
                && gs.ReadEventBool(0x16cb)
                && gs.ReadEventBool(0x16ce)
                && gs.ReadEventBool(0x16cf)
                && gs.ReadEventBool(0x16d1))
            {
                gs.SetEventValue(sEncounterFlag_1d17, 1);
            }
        default:break;
    }
}

void ZeroCombatClicked(unsigned combatIndex)
{
    auto offset = (combatIndex * 0xe) + 0x131f;
}

void ReactivateCombat(
    FileBuffer& fb,
    ZoneNumber zone,
    const Encounter::Encounter& encounter,
    CombatIndex combatIndex)
{
    const auto encounterIndex = encounter.GetIndex().mValue;
    auto tileIndex = encounter.GetTileIndex();
    SetUniqueEncounterStateFlag(fb, zone, tileIndex, encounterIndex, false);
    SetRecentlyEncountered(fb, encounterIndex);
    SetCombatEncounterScoutedState(fb, encounterIndex, false);
    SetCombatEncounterState(fb, combatIndex, false);
    SetCombatClickedTime(fb, combatIndex, Time{0});
}

void DeactivateCombat(
    FileBuffer& fb,
    ZoneNumber zone,
    const Encounter::Encounter& encounter,
    CombatIndex combatIndex)
{
    SetCombatEncounterState(fb, combatIndex, true);
}

Time GetCombatClickedTime(FileBuffer& fb, CombatIndex combatIndex)
{
    static constexpr auto offset = 0x4457 + 0x64;
    fb.Seek(offset + (combatIndex.mValue << 2));
    return Time{fb.GetUint32LE()};
}

void SetCombatClickedTime(FileBuffer& fb, CombatIndex combatIndex, Time time)
{
    static constexpr auto offset = 0x4457 + 0x64;
    fb.Seek(offset + (combatIndex.mValue << 2));
    fb.PutUint32LE(time.mTime);
}

}
