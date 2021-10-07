#pragma once

#include "bak/dialog.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"
#include "bak/gameData.hpp"
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
        mGameData{gameData}
    {}

    struct Character
    {
        std::string mName;
        unsigned mIndex;
    };

    Chapter GetChapter() const
    {
        return 1;
    }

    auto GetMoney() const
    {
        if (mGameData)
            return mGameData->mParty.mGold;

        return 1000;
    }

    auto GetZone() const
    {
        if (mGameData)
            return ZoneNumber{mGameData->mLocation.mZone};
        return ZoneNumber{1};
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

    bool EvaluateGameStateChoice(const GameStateChoice& choice) const
    {
        Logging::LogDebug(__FUNCTION__) << "Choice: " 
            << Choice{choice} << " time: " << GetTime() << "\n";
        if (choice.mState == BAK::ActiveStateFlag::Chapter
            && (GetChapter() >= choice.mExpectedValue
                && GetChapter() <= choice.mExpectedValue2))
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::Money
            && GetMoney() > (choice.mExpectedValue * 10))
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
                    // FIXME: Fill this in...
                    return false;
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

    Character mPartyLeader;
    Character mPartyFollower;
    GameData* mGameData;
    std::unordered_map<unsigned, bool> mEventState;
};

}
