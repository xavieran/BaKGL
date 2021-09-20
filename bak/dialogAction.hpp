#pragma once

#include "bak/condition.hpp"
#include "bak/skills.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/worldClock.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace BAK {

enum class DialogResult
{
    // Maybe also something to do wih tstate?
    SetTextVariable = 0x01,
    GiveItem        = 0x02,
    LoseItem        = 0x03, // pt2 object, pt3 amount
    // Unlocks dialog options / Sets event state
    SetFlag         = 0x04,
    Unknown5        = 0x05,
    // For popup dialogs sets the dimensions of the dialog
    SetPopupDimensions = 0x06,
    // e.g. sickness when visiting orno (2f4e8c)
    GainCondition = 0x08,
    GainSkill  = 0x09,
    PlaySound  = 0x0c,
    ElapseTime = 0x0d,
    // Push this Key/Offset to the dialog queue
    // i.e. when the current dialog is finished, we enter this dialog
    PushNextDialog = 0x10,
    // Transition to another location? e.g. sewer dialog 231861
    Transition = 0x14,
    // 18 - seems to remove/move mney at a chapter transition
    // 17 - maybe moves party members around? e.g. at chapter transition
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
    std::uint16_t mFlag;
    Condition mCondition;
    // Value 1 seems to be the one that actually takes effect
    // if value1 == 0xff9c then it reverses/heals the specific condition
    std::int16_t mValue1;
    std::int16_t mValue2;
};

struct GainSkill
{
    std::uint16_t mFlag;
    SkillType mSkill;
    // These may be different and I'm not sure on their meaning when different
    std::int8_t mValue0;
    std::int8_t mValue1;
    std::int8_t mValue2;
    std::int8_t mValue3;
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
    LoseItem,
    GiveItem,
    SetFlag,
    ElapseTime,
    SetPopupDimensions,
    GainCondition,
    GainSkill,
    PushNextDialog,
    UnknownAction>;

std::ostream& operator<<(std::ostream& os, const DialogAction& d);

}
