#pragma once

namespace BAK::State {


static constexpr auto sCharacterSelectedSkillPool = 0x324; // -> 0x319
                                                           //
static constexpr auto sSkillSelectedEventFlag = 0x1856;
static constexpr auto sSkillImprovementEventFlag = 0x18ce;

// Single bit indicators for event state tracking 
// In the code this offset is 0x440a in the game -> diff of 0x3d28
static constexpr auto sGameEventRecordOffset = 0x6e2; // -> 0xadc
static constexpr auto sGameComplexEventRecordOffset = 0xb09;

static constexpr auto sConversationChoiceMarkedFlag = 0x1d4c;
static constexpr auto sConversationOptionInhibitedFlag = 0x1a2c;

static constexpr auto sLockHasBeenSeenFlag = 0x1c5c;

}
