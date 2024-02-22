#include "bak/state/encounter.hpp"

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
    const bool eventFlag1 = encounter.mSaveAddress != 0
        ? !gs.GetEventState(CreateChoice(encounter.mSaveAddress))
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mSaveAddress2 != 0
        ? gs.GetEventState(CreateChoice(encounter.mSaveAddress2))
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
    const bool eventFlag1 = encounter.mSaveAddress != 0
        ? !gs.GetEventState(CreateChoice(encounter.mSaveAddress))
        : false;
    // event flag 2 - this flag must _not_ be set to encounter this event
    const bool eventFlag2 = encounter.mSaveAddress2 != 0
        ? gs.GetEventState(CreateChoice(encounter.mSaveAddress2))
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
    constexpr auto encounterStateOffset = 0x190;
    constexpr auto maxEncountersPerTile = 0xa;
    const auto zoneOffset = (zone.mValue - 1) * encounterStateOffset;
    const auto tileOffset = tileIndex * maxEncountersPerTile;
    const auto offset = zoneOffset + tileOffset + encounterIndex;
    return offset + encounterStateOffset;
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
    const GameState& gs,
    unsigned combatIndex) 
{
    constexpr auto alwaysTriggeredIndex = 0x3e8;
    if (combatIndex >= alwaysTriggeredIndex)
        return true;
    else
        return gs.ReadEventBool(
            CalculateCombatEncounterStateFlag(combatIndex));
}

void SetCombatEncounterState(
    FileBuffer& fb,
    unsigned combatIndex,
    bool state)
{
    constexpr auto alwaysTriggeredIndex = 0x3e8;
    if (combatIndex >= alwaysTriggeredIndex)
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
    constexpr auto offset = 0x1450;
    return offset + encounterIndex;
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

void SetPostCombatCombatSpecificFlags(GameState& gs, unsigned combatIndex)
{
    switch (combatIndex)
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
        // The never-ending combats?
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
            // Zero State 0x190  UniqueEncounterStateFlag
            // Zero state 0x1450 Recently encountered encounter
            // Zero state 145a
            // Zero state 1464
            // Zero combat clicked...
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
                gs.SetEventValue(0x1d17, 1);
            }
        default:break;
    }
}

void ZeroCombatClicked(unsigned combatIndex)
{
    auto offset = (combatIndex * 0xe) + 0x131f;
}

Time GetCombatClickedTime(FileBuffer& fb, unsigned combatIndex)
{
    static constexpr auto offset = 0x4457 + 0x64;
    fb.Seek(offset + (combatIndex << 2));
    return Time{fb.GetUint32LE()};
}

void SetCombatClickedTime(FileBuffer& fb, unsigned combatIndex, Time time)
{
    static constexpr auto offset = 0x4457 + 0x64;
    fb.Seek(offset + (combatIndex << 2));
    fb.PutUint32LE(time.mTime);
}

}
