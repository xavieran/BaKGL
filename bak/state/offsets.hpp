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


}
