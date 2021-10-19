#pragma once

#include "bak/dialog.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"
#include "bak/gameData.hpp"
#include "bak/money.hpp"
#include "bak/types.hpp"

#include "com/visit.hpp"

namespace BAK {

class GameState
{
public:
    GameState()
    :
        GameState{nullptr}
    {}

    GameState(
        GameData* gameData)
    :
        mPartyLeader{
            "Locklear",
            1},
        mPartyFollower{
            "Owyn",
            2},
        mGameData{gameData},
        mParty{
            Royals{1000},
            Inventory{},
            {},
            {}},
        mContextValue{0},
        mSkillValue{0},
        mChapter{1},
        mZone{1},
        mLogger{Logging::LogState::GetLogger("BAK::GameState")}
    {}

    const Party& GetParty() const
    {
        if (mGameData)
            return mGameData->mParty;
        return mParty;
    }

    Party& GetParty()
    {
        if (mGameData)
            return mGameData->mParty;
        return mParty;
    }

    struct Character
    {
        std::string mName;
        unsigned mIndex;
    };

    void SetChapter(Chapter chapter)
    {
        if (mGameData)
            mGameData->mChapter = chapter;
        else mChapter = chapter;
    }

    Chapter GetChapter() const
    {
        if (mGameData)
            return mGameData->mChapter;

        return mChapter;
    }

    Royals GetMoney() const
    {
        if (mGameData)
            return mGameData->mParty.GetGold();

        return Royals{1000};
    }

    void SetLocation(BAK::Location loc)
    {
        if (mGameData)
            mGameData->mLocation = loc;
        else
            mZone = ZoneNumber{loc.mZone};
    }

    auto GetZone() const
    {
        if (mGameData)
            return ZoneNumber{mGameData->mLocation.mZone};
        return mZone;
    }

    auto GetTime() const
    {
        if (mGameData)
        {
            const auto hour = mGameData->mTime.mTime.GetHour();
            return 0;
            //return static_cast<int>(hour < 18);
        }
        else
        {
            return 0; // daytime
            //return 1; // nighttime
        }
    }

    auto GetShopType() const
    {
        return 4;
    }

    const Character& GetPartyLeader()
    {
        return mPartyLeader;
    }

    // Return random person who's not the leader...
    const Character& GetPartyFollower()
    {
        return mPartyFollower;
    }

    void EvaluateAction(const DialogAction& action)
    {
        std::visit(overloaded{
            [&](const BAK::SetFlag& set)
            {
                mLogger.Debug() << "Setting flag of event: " << BAK::DialogAction{set} << "\n";
                SetEventState(set);
            },
            [&](const BAK::LoseItem& lose)
            {
                // FIXME: Implement if this skill only improves for one character
                //        Figure out what to do with mValue0,2,3 
                auto& party = GetParty();
                party.RemoveItem(lose.mItemIndex, lose.mQuantity);
            },
            [&](const BAK::GiveItem& give)
            {
                // FIXME: Implement if this skill only improves for one character
                //        Figure out what to do with mValue0,2,3 
                auto& party = GetParty();
                party.GainItem(give.mCharacter, give.mItemIndex, give.mQuantity);
            },
            [&](const BAK::GainSkill& skill)
            {
                // FIXME: Implement if this skill only improves for one character
                //        Figure out what to do with mValue0,2,3 
                auto& party = GetParty();
                for (const auto c : party.mActiveCharacters)
                {
                    party.mCharacters[c].mSkills.ImproveSkill(
                        skill.mSkill,
                        skill.mValue1);
                }
            },
            [&](const BAK::GainCondition& cond)
            {
                // FIXME: Implement if this condition only changes for one character
                auto& party = GetParty();
                for (const auto c : party.mActiveCharacters)
                {
                    party.mCharacters[c].mConditions.IncreaseCondition(
                        cond.mCondition, cond.mValue1);
                }
            },
            [&](const BAK::LoadSkillValue& load)
            {
                const auto [character, value] = GetParty().GetSkill(load.mSkill, load.mTarget == 1);
                mSkillValue = value;
            },
            [&](const auto& a){
                mLogger.Debug() << "Doing nothing for: " << a << "\n";
            }},
            action);
    }

    bool EvaluateGameStateChoice(const GameStateChoice& choice) const
    {
        Logging::LogDebug(__FUNCTION__) << "Choice: " 
            << Choice{choice} << " contextVar: " << mContextValue << " time: " << GetTime() << "\n";
        if (choice.mState == BAK::ActiveStateFlag::Chapter
            && (GetChapter() >= choice.mExpectedValue
                && GetChapter() <= choice.mExpectedValue2))
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::Context
            && mContextValue == choice.mExpectedValue)
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::SkillCheck
            && mSkillValue >= choice.mExpectedValue)
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::Money
            && (GetMoney().mValue > GetRoyals(Sovereigns{static_cast<unsigned>(choice.mExpectedValue)}).mValue))
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::NightTime
            && GetTime() == choice.mExpectedValue)
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::Shop
            && GetShopType() == choice.mExpectedValue)
        {
            return true;
        }

        return false;
    }

    bool EvaluateComplexChoice(const ComplexEventChoice& choice) const
    {
        const auto state = GetComplexEventState(choice.mEventPointer);
        const auto expectedValue = choice.mExpectedValue;
        return ((state ^ choice.mXorWith) == expectedValue);
        // mUnknown1 seems to be a mask over the XOR mask
        // mUnknown2 seems to be a mask over the value that was read...
        // Not 100% sure on this need to do more trials
    }

    bool EvaluateDialogChoice(const Choice& choice) const
    {
        return std::visit(overloaded{
                [&](const EventFlagChoice& c){
                    return GetEventState(c.mEventPointer) == c.mExpectedValue;
                },
                [&](const ComplexEventChoice& c){
                    const auto state = GetComplexEventState(c.mEventPointer);
                    if (c.mUnknown1 == 1)
                        return state != c.mExpectedValue;
                    else
                        return state == c.mExpectedValue;
                    //return (state ^ c.mXorWith) == c.mExpectedValue;
                },
                [&](const InventoryChoice& c){
                    return GetParty().HaveItem(c.mRequiredItem) == c.mItemPresent;
                },
                [&](const GameStateChoice& c){
                    return EvaluateGameStateChoice(c);
                },
                [&](const auto& c){
                    return true; 
                },
            },
            choice);
    }

    bool GetComplexEventState(unsigned eventPtr) const
    {
        if (mEventState.contains(eventPtr))
            return mEventState.at(eventPtr);
        else if (mGameData != nullptr)
            return mGameData->ReadComplexEvent(eventPtr);
        else
            return false;
    }

    bool GetEventState(unsigned eventPtr) const
    {
        if (mEventState.contains(eventPtr))
            return mEventState.at(eventPtr);
        else if (mGameData != nullptr)
            return mGameData->ReadEvent(eventPtr);
        else
            return false;
    }

    bool CheckInhibited(const ConversationChoice& choice)
    {
        if (mEventState.contains(choice.mEventPointer + 0x1a2c))
        {
            return mEventState.at(choice.mEventPointer + 0x1a2c);
        }
        else if (mGameData != nullptr)
        {
            return mGameData->CheckConversationOptionInhibited(
                choice.mEventPointer);
        }
        return false;
    }

    bool CheckDiscussed(const ConversationChoice& choice)
    {
        if (mEventState.contains(choice.mEventPointer + 0xa8c))
        {
            return mEventState.at(choice.mEventPointer + 0xa8c);
        }
        else if (mGameData != nullptr)
        {
            return mGameData->ReadConversationItemClicked(
                choice.mEventPointer);
        }
        return false;
    }

    void MarkDiscussed(const ConversationChoice& choice)
    {
        mEventState.emplace(choice.mEventPointer + 0xa8c, true);
    }

    void SetEventState(const SetFlag& setFlag)
    {
        // All complex events have 0xd000 as first byte
        if ((setFlag.mEventPointer & 0xd000) == 0xd000)
        {
            SetComplexEvent(setFlag);
        }
        else
        {
            if (mGameData)
                mGameData->SetEventFlag(true, setFlag.mEventPointer);
            else
                mEventState.emplace(setFlag.mEventPointer, true);
        }
    }

    void SetComplexEvent(const SetFlag& setFlag)
    {
        const auto data = GetEventState(setFlag.mEventPointer);
        const auto result = (data & setFlag.mEventMask) 
            | setFlag.mEventData;
        mEventState.emplace(setFlag.mEventPointer, result);
    }

    void SetDialogContext(unsigned contextValue)
    {
        mContextValue = contextValue;
    }

    void ClearUnseenImprovements(unsigned character)
    {
        if (mGameData)
        {
            mGameData->ClearUnseenImprovements(character);
        }

    }

    Character mPartyLeader;
    Character mPartyFollower;
    GameData* mGameData;
    Party mParty;
    unsigned mContextValue;
    unsigned mSkillValue;
    unsigned mChapter;
    ZoneNumber mZone;
    std::unordered_map<unsigned, bool> mEventState;
    const Logging::Logger& mLogger;
};

}
