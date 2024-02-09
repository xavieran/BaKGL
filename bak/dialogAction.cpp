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
        case DialogResult::SetPopupDimensions: return os << "SetPopupDimensions";
        case DialogResult::SpecialAction: return os << "SpecialAction";
        case DialogResult::GainCondition: return os << "GainCondition";
        case DialogResult::GainSkill: return os << "GainSkill";
        case DialogResult::LoadSkillValue: return os << "LoadSkillValue";
        case DialogResult::PlaySound: return os << "PlaySound";
        case DialogResult::ElapseTime: return os << "ElapseTime";
        case DialogResult::SetAddResetState: return os << "SetAddResetState";
        case DialogResult::PushNextDialog: return os << "PushNextDialog";
        case DialogResult::UpdateCharacters: return os << "UpdateCharacters";
        case DialogResult::HealCharacters: return os << "HealCharacters";
        case DialogResult::LearnSpell: return os << "LearnSpell";
        case DialogResult::Teleport: return os << "Teleport";
        case DialogResult::SetEndOfDialogState: return os << "SetEndOfDialogState";
        case DialogResult::SetTimeExpiringState: return os << "SetTimeExpiringState";
        case DialogResult::LoseItem2: return os << "LoseItem2";
        default: return os << "Unknown[" << static_cast<unsigned>(d) << "]";
    }
}

std::ostream& operator<<(std::ostream& os, const SpecialActionType& a)
{
    switch (a)
    {
        case SpecialActionType::ReduceGold: return os << "ReduceGold";
        case SpecialActionType::IncreaseGold: return os << "IncreaseGold";
        case SpecialActionType::DoesSomethingWithItemsOfEachChar: return os << "DoesSomethingWithItemsOfEachChar";
        case SpecialActionType::ResetCombatState: return os << "ResetCombatState";
        case SpecialActionType::SetCombatState: return os << "SetCombatState";
        case SpecialActionType::DoSomethingWithContainer0: return os << "DoSomethingWithContainer0";
        case SpecialActionType::DoSomethingWithContainer1: return os << "DoSomethingWithContainer1";
        case SpecialActionType::Increase753f: return os << "Increase753f";
        case SpecialActionType::Gamble: return os << "Gamble";
        case SpecialActionType::CharInventoryP: return os << "CharInventoryP";
        case SpecialActionType::DoSomethingWithContainer2: return os << "DoSomethingWithContainer2";
        case SpecialActionType::ResetGambleValueTo: return os << "ResetGambleValueTo";
        case SpecialActionType::BeginCombat: return os << "BeginCombat";
        case SpecialActionType::ExpireAllTimeExpiringStateP: return os << "ExpireAllTimeExpiringStateP";
        case SpecialActionType::ArlieSteelSoulContainer: return os << "ArlieSteelSoulContainer";
        case SpecialActionType::CheatIncreaseSkill: return os << "CheatIncreaseSkill";
        case SpecialActionType::AfterPugAddedToParty: return os << "AfterPugAddedToParty";
        default:
            return os << "Unknown[" << static_cast<unsigned>(a) <<"]";
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
    os << "ElapseTime { HowLong: " << action.mTime
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Teleport& action)
{
    os << "Teleport{ Index: " << action.mIndex << " }";
    return os;
}

std::ostream& operator<<(std::ostream& os, const UpdateCharacters& action)
{
    os << "UpdateCharacters { chars: " << action.mNumberChars
        << " | " << action.mCharacter0 << " " << action.mCharacter1 
        << " " << action.mCharacter2 << "}";
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

std::ostream& operator<<(std::ostream& os, const GainCondition& action)
{
    os << "GainCondition{ who: " << action.mWho << " " << ToString(action.mCondition)
        << " [" << action.mMin << ", " << action.mMax << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GainSkill& action)
{
    os << "GainSkill{ who: " << action.mWho << " " << ToString(action.mSkill)
        << " [" << +action.mMin << ", " << +action.mMax << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SpecialAction& action)
{
    os << "SpecialAction{ Type: " << action.mType << " ["
        << action.mVar1 << ", " << action.mVar2 << ", " 
        << action.mVar3 << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LoadSkillValue& action)
{
    os << "LoadSkillValue{ target: " << action.mTarget 
        << " " << ToString(action.mSkill) << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const PlaySound& action)
{
    os << "PlaySound { index: " << action.mSoundIndex
        << " flag: " << action.mFlag << " " << action.mRest << "}";
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

std::ostream& operator<<(std::ostream& os, const SetAddResetState& action)
{
    os << "SetAddResetState{" << std::hex << action.mEventPtr << " unk0: " << action.mUnknown0
        << " TimeToExpire: " << std::dec << action.mTimeToExpire << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SetTimeExpiringState& action)
{
    os << "SetTimeExpiringState{" << std::hex << " type: " << action.mType
        << " flags: " << +action.mFlags
        << " eventPtr: " << action.mEventPtr
        << " TimeToExpire: " << std::dec << action.mTimeToExpire << "}";
    return os;
}


std::ostream& operator<<(std::ostream& os, const HealCharacters& action)
{
    os << "HealCharacters{who: " << action.mWho
        << " howMuch: " << action.mHowMuch << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const SetEndOfDialogState& action)
{
    os << "SetEndOfDialogState{" << action.mState
        << " rest[" << std::hex << action.mRest << std::dec << "]}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LearnSpell& action)
{
    os << "LearnSpell{who: " << action.mWho << ", whichSpell: " << action.mWhichSpell << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LoseItem2& action)
{
    os << "LoseItem2 { what: " << action.mItemIndex
        << " amount: " << action.mQuantity
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
