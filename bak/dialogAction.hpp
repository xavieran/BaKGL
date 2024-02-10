#pragma once

#include "bak/condition.hpp"
#include "bak/timeExpiringState.hpp"
#include "bak/types.hpp"
#include "bak/skills.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/worldClock.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace BAK {

enum class DialogResult
{
    SetTextVariable = 0x01,
    GiveItem        = 0x02,
    LoseItem        = 0x03,
    SetFlag         = 0x04,
    LoadActor        = 0x05,
    SetPopupDimensions = 0x06,
    SpecialAction = 0x07,
    GainCondition = 0x08,
    GainSkill  = 0x09,
    LoadSkillValue = 0x0a,
    //PlaySound2  = 0x0b, unused, but calls same code as play sound...
    PlaySound  = 0x0c,
    ElapseTime = 0x0d,
    SetAddResetState = 0x0e,
    // This seems irrelevant to us - calls a function which
    // seems to reset some state and free memory
    FreeMemoryP = 0x0f,
    PushNextDialog = 0x10,
    UpdateCharacters = 0x11,
    HealCharacters   = 0x12,
    LearnSpell = 0x13,
    Teleport = 0x14,
    SetEndOfDialogState = 0x15,
    SetTimeExpiringState = 0x16,
    LoseItem2 = 0x17
    // 18 - seems to remove/move mney at a chapter transition
};

enum class SpecialActionType : std::uint16_t
{
    ReduceGold = 0, // reduces party gold by val of item or to 0 if lt
    IncreaseGold = 1,
    DoesSomethingWithItemsOfEachChar = 2,
    ResetCombatState = 3,
    SetCombatState = 4,
    DoSomethingWithContainer0 = 5,
    DoSomethingWithContainer1 = 6,
    Increase753f = 7, // this literally adds the value to 753f
    Gamble = 8,
    CharInventoryP = 9, // this does something funky with inventory
    DoSomethingWithContainer2 = 10,
    ResetGambleValueTo = 11,
    BeginCombat = 12,
    ExpireAllTimeExpiringStateP = 13, // seems to expire TESs
    ArlieSteelSoulContainer = 14,
    CheatIncreaseSkill = 15,
    AfterPugAddedToParty = 16, // modifies vars at 3ec3 and 3f22
};

struct SetTextVariable
{
    // Sets the variable (@mWhich) to mWhat
    std::uint16_t mWhich;

    // mWhat:
    // 0x0b - selected party member (e.g. give item to b)
    // 0x10 - other party member?
    // 0x11 - monster ?
    // 0x12 - chosen item (e.g. in bless screen choose a sword)
    // 0x13 - shop cost (e.g. in bless screen cost of blessing)
    // 0x14 - party wallet (e.g. in bless screen amount of cash)
    // 0x1c - shop attendant name (e.g. shop keeper)
    // 0x1d - Stat? e.g. health
    std::uint16_t mWhat;
    std::array<std::uint8_t, 4> mRest;
};

struct LoseItem
{
    std::uint16_t mItemIndex;
    std::uint16_t mQuantity;
    std::array<std::uint8_t, 4> mRest;
};

struct GiveItem
{
    std::uint8_t mItemIndex;
    // if 2 then give to text variable set character
    std::uint8_t mCharacter; 
    std::uint16_t mQuantity;
    std::array<std::uint8_t, 4> mRest;
};

// single bit flags
// 1 -> ab (these are for dialog choices)
// and
// 1a2d -> 1fd3
// 104 (seems to be set to 0 when short of money?)
struct SetFlag
{
    std::uint16_t mEventPointer;
    std::uint8_t mEventMask;
    std::uint8_t mEventData;
    std::uint16_t mAlwaysZero;
    std::uint16_t mEventValue;
};

// Unclear what this does. Modifying it in the DIAL_Z** files
// and running the game seems to do nothing. It is definitely
// related to the actor portrait index, but unclear how.
// Maybe preloads palettes and images????
struct LoadActor
{
    std::uint16_t mActor1;
    std::uint16_t mActor2;
    std::uint16_t mActor3;
    std::uint16_t mUnknown;
};

struct SetPopupDimensions
{
    glm::vec2 mPos;
    glm::vec2 mDims;
};

struct GainCondition
{
    // if flag == 0 or 1 affects all
    // 2 affects person who was set by "SetTextVariable"
    // 8, 9 - locklear
    // if flag == 7 gorath last person in party?
    // if flag == 6 locklear party leader.. ?
    // if flag == 5 owyn second party member?
    //
    std::uint16_t mWho;
    Condition mCondition;
    std::int16_t mMin;
    std::int16_t mMax;
};

struct GainSkill
{
    std::uint16_t mWho;
    SkillType mSkill;
    std::int16_t mMin;
    std::int16_t mMax;
};

struct SpecialAction
{
    SpecialActionType mType;
    std::uint16_t mVar1;
    std::uint16_t mVar2;
    std::uint16_t mVar3;
};

struct LoadSkillValue
{
    std::uint16_t mTarget;
    SkillType mSkill;
};

struct PlaySound
{
    std::uint16_t mSoundIndex;
    std::uint16_t mFlag;
    std::uint32_t mRest;
};

struct ElapseTime
{
    Time mTime;
    std::array<std::uint8_t, 4> mRest;
};

struct PushNextDialog
{
    BAK::Target mTarget;
    std::array<std::uint8_t, 4> mRest;
};

struct UpdateCharacters
{
    std::uint16_t mNumberChars;
    CharIndex mCharacter0;
    CharIndex mCharacter1;
    CharIndex mCharacter2;
};

struct Teleport
{
    TeleportIndex mIndex;
};

struct SetAddResetState
{
    std::uint16_t mEventPtr;
    std::uint16_t mUnknown0;
    Time mTimeToExpire;
};

struct HealCharacters
{
    std::uint16_t mWho;
    std::uint16_t mHowMuch;
};

struct SetEndOfDialogState
{
    std::int16_t mState;
    std::array<std::uint8_t, 6> mRest;
};

struct LearnSpell
{
    LearnSpell(
        std::uint16_t who,
        std::uint16_t whichSpell)
    :
        mWho(who),
        mWhichSpell(whichSpell)
    {}

    std::uint16_t mWho;
    SpellIndex mWhichSpell;
};

struct SetTimeExpiringState
{
    ExpiringStateType mType;
    std::uint8_t mFlags;
    std::uint16_t mEventPtr;
    Time mTimeToExpire;
};

struct LoseItem2
{
    std::uint16_t mItemIndex;
    std::uint16_t mQuantity;
    std::array<std::uint8_t, 4> mRest;
};

struct UnknownAction
{
    UnknownAction(
        std::uint16_t type,
        const std::array<std::uint8_t, 8>& rest)
    :
        mType{static_cast<DialogResult>(type)},
        mRest{rest}
    {}

    DialogResult mType;
    std::array<std::uint8_t, 8> mRest;
};

using DialogAction = std::variant<
    SetTextVariable,
    GiveItem,
    LoseItem,
    SetFlag,
    LoadActor,
    SetPopupDimensions,
    SpecialAction,
    GainCondition,
    GainSkill,
    LoadSkillValue,
    PlaySound,
    ElapseTime,
    SetAddResetState,
    PushNextDialog,
    UpdateCharacters,
    HealCharacters,
    LearnSpell,
    Teleport,
    SetEndOfDialogState,
    SetTimeExpiringState,
    LoseItem2,
    UnknownAction>;

std::ostream& operator<<(std::ostream& os, const DialogAction& d);

}
