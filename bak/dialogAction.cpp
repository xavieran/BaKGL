#include "bak/dialogAction.hpp"

#include "com/ostream.hpp"

#include "graphics/glm.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const DialogResult& d)
{
    switch (d)
    {
        case DialogResult::SetTextVariable: return os << "SetTextVariable";
        case DialogResult::GiveItem: return os << "GiveItem";
        case DialogResult::LoseItem: return os << "LoseItem";
        case DialogResult::SetFlag: return os << "SetFlag";
        case DialogResult::Unknown5: return os << "Unknown[5]";
        case DialogResult::GainCondition: return os << "GainCondition";
        case DialogResult::GainSkill: return os << "GainSkill";
        case DialogResult::LoadSkillValue: return os << "LoadSkillValue";
        case DialogResult::PlaySound: return os << "PlaySound";
        case DialogResult::ElapseTime: return os << "ElapseTime";
        case DialogResult::PushNextDialog: return os << "PushNextDialog";
        case DialogResult::Transition: return os << "Transition";
        default: return os << "(" << static_cast<unsigned>(d) << ")";
    }
}

std::ostream& operator<<(std::ostream& os, const SetTextVariable& action)
{
    os << "SetTextVariable { var: " << action.mWhich << ", set to: " << action.mWhat 
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ElapseTime& action)
{
    os << "ElapseTime { HowLong: " << action.mTime.ToString()
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Transition& action)
{
    os << "Transition{ Index: " << action.mIndex << " }";
    return os;
}


std::ostream& operator<<(std::ostream& os, const LoseItem& action)
{
    os << "LoseItem { what: " << action.mItemIndex 
        << " amount: " << action.mQuantity
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GiveItem& action)
{
    os << "GiveItem { what: " << +action.mItemIndex << " to: "
        << +action.mCharacter << " amount: " << +action.mQuantity
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SetFlag& action)
{
    os << "SetFlag {" << std::hex << action.mEventPointer
        << " mask: " << +action.mEventMask << " data: " << +action.mEventData
        << " z: " << action.mAlwaysZero << " val: " << action.mEventValue
        << std::dec << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GainCondition& cond)
{
    os << "GainCondition{ who: " << cond.mFlag << " " << ToString(cond.mCondition)
        << " Val1: " << cond.mValue1 << " Val2: " << cond.mValue2 << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GainSkill& cond)
{
    os << "GainSkill{ who: " << cond.mFlag << " " << ToString(cond.mSkill)
        << " [" << +cond.mValue0 
        << ", " << +cond.mValue1 
        << ", " << +cond.mValue2 
        << ", " << +cond.mValue3 << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LoadSkillValue& action)
{
    os << "LoadSkillValue{ target: " << action.mTarget 
        << " " << ToString(action.mSkill) << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SetPopupDimensions& action)
{
    os << "SetPopupDimensions { pos: " << action.mPos << ", dims: " << action.mDims << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PushNextDialog& action)
{
    os << "PushNextDialog {" << action.mTarget
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const UnknownAction& action)
{
    os << "UnknownAction { " 
        << " type: " << action.mType
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const DialogAction& action)
{
    std::visit([&os](const auto& a){
            os << a;
        },
        action);

    return os;
}

}
