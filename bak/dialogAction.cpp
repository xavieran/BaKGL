#include "bak/dialogAction.hpp"

#include "com/ostream.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const DialogResult& d)
{
    switch (d)
    {
        case DialogResult::Unknown1: return os << "Unknown[1]";
        case DialogResult::GiveItem: return os << "GiveItem";
        case DialogResult::LoseItem: return os << "LoseItem";
        case DialogResult::SetFlag: return os << "SetFlag";
        case DialogResult::Unknown5: return os << "Unknown[5]";
        case DialogResult::GiveCondition: return os << "GiveCondition";
        case DialogResult::GainSkill: return os << "GainSkill";
        case DialogResult::PlaySound: return os << "PlaySound";
        case DialogResult::ElapseTime: return os << "ElapseTime";
        case DialogResult::PushNextDialog: return os << "PushNextDialog";
        case DialogResult::Transition: return os << "Transition";
        default: return os << "(" << static_cast<unsigned>(d) << ")";
    }
}
std::ostream& operator<<(std::ostream& os, const PushNextDialog& action)
{
    os << "PushNextDialog {" << action.mTarget << ", rest[" << action.mRest << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SetFlag& action)
{
    os << "SetFlag {" << action.mEventPointer << ", rest[" << action.mRest << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const UnknownAction& action)
{
    os << "DA { " 
        << " type: " << action.mType
        << " rst: [" << action.mRest << "]}";
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
