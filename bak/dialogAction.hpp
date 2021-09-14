#pragma once

#include "bak/dialogTarget.hpp"

#include <variant>


namespace BAK {

enum class DialogResult
{
    // Maybe also something to do wih tstate?
    Unknown1   = 0x01,
    GiveItem   = 0x02,
    LoseItem   = 0x03, // pt2 object, pt3 amount
    // Unlocks dialog options / Sets event state
    SetFlag    = 0x04,
    Unknown5   = 0x05,
    // e.g. sickness when visiting orno (2f4e8c)
    GiveCondition = 0x08,
    GainSkill  = 0x09,
    PlaySound  = 0x0c,
    ElapseTime = 0x0d,
    // Push this Key/Offset to the dialog queue
    // i.e. when the current dialog is finished, we enter this dialog
    PushNextDialog = 0x10,
    // Transition to another location? e.g. sewer dialog 231861
    Transition = 0x14,
};

struct PushNextDialog
{
    BAK::Target mTarget;
    std::array<std::uint8_t, 4> mRest;
};

struct SetFlag
{
    std::uint16_t mEventPointer;
    std::array<std::uint8_t, 6> mRest;
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
    PushNextDialog,
    SetFlag,
    UnknownAction>;

std::ostream& operator<<(std::ostream& os, const DialogAction& d);

}
