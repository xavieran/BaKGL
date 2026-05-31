#pragma once

namespace BAK::State {


static constexpr auto sCharacterSelectedSkillPool = 0x324; // -> 0x319
                                                           //
static constexpr auto sSkillSelectedEventFlag = 0x1856;
static constexpr auto sSkillImprovementEventFlag = 0x18ce;
static constexpr auto sConditionStateEventFlag = 0x1c98;
// e.g. books
static constexpr auto sItemUsedForCharacterAtLeastOnce = 0x194c;
static constexpr auto sSpynoteHasBeenRead = 0x1964;

// Single bit indicators for event state tracking 
// In the code this offset is 0x440a in the game -> diff of 0x3d28
static constexpr auto sGameEventRecordOffset = 0x6e2; // -> 0xadc
static constexpr auto sGameComplexEventRecordOffset = 0xb09;

static constexpr auto sConversationChoiceMarkedFlag = 0x1d4c;
static constexpr auto sConversationOptionInhibitedFlag = 0x1a2c;

static constexpr auto sLockHasBeenSeenFlag = 0x1c5c;

// Based on disassembly this may be the state of doors (open/closed)
static constexpr auto sDoorFlag = 0x1b58;

// Encounter state flag range
static constexpr auto sEncounterStateOffset = 0x190;
static constexpr auto sEncounterStateCount = 0x12c0;

// Encounter sub-ranges
static constexpr auto sRecentEncounterOffset = 0x1450;
static constexpr auto sCombatScoutedOffset = 0x145a;
static constexpr auto sCombatEncounterOffset = 0x1464;

// Maximum combat index before always-triggered
static constexpr auto sMaxCombatIndex = 0x3e8;

// Per-character sizes for skill and item tracking
static constexpr auto sMaxSkills = 0x11;
static constexpr auto sMaxTrackedItems = 0x14;

// Temple base (moved from temple.cpp local)
static constexpr auto sTempleSeenFlag = 0x1950;

// Max standard event pointer (one-past-end, derived from buffer layout)
static constexpr auto sMaxStandardEventPtr = 0x2140;

// Named individual event flags
static constexpr auto sQuestFlag_1972 = 0x1972;
static constexpr auto sQuestFlag_1979 = 0x1979;
static constexpr auto sChapterTransitionFlag_1ab1 = 0x1ab1;
static constexpr auto sEncounterFlag_1d17 = 0x1d17;
static constexpr auto sQuestFlag_1f6c = 0x1f6c;
static constexpr auto sQuestFlag_1fbc = 0x1fbc;

}
