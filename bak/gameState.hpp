#pragma once

#include "bak/container.hpp"
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
            Inventory{20},
            {},
            {}},
        mContextValue{0},
        mSkillValue{0},
        mChapter{1},
        mZone{1},
        mContainers{},
        mGDSContainers{},
        mLogger{Logging::LogState::GetLogger("BAK::GameState")}
    {
        if (mGameData != nullptr)
        {
            for (unsigned i = 0; i < 12; i++)
            {
                mContainers.emplace_back(
                    mGameData->LoadContainers(i + 1));
            }
            mGDSContainers = mGameData->LoadShops();
        }
    }

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

    IContainer* GetContainerForGDSScene(BAK::HotspotRef ref)
    {
        for (auto& shop : mGDSContainers)
        {
            if (shop.mGdsScene == ref)
                return &shop;
        }
        return nullptr;
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
                    party.mCharacters[c.mValue].mSkills.ImproveSkill(
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
                    party.mCharacters[c.mValue].mConditions.IncreaseCondition(
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
        mLogger.Debug() << __FUNCTION__ << "Choice: " << choice 
            << " S: [" << std::hex << +state << std::dec << "]\n";

        const auto chapterFlag = GetChapter() == 9
            ? 0x80
            : 1 << (GetChapter() - 1);
        const auto chapterMaskSatisfied
            = (chapterFlag & choice.mChapterMask) != 0;

        if (choice.mMustEqualExpected)
        {
            if (((state ^ choice.mXorMask) & choice.mExpected) == choice.mExpected
                && chapterMaskSatisfied)
                return true;
            else
                return false;
        }
        else
        {
            if (((state ^ choice.mXorMask) & choice.mExpected) != 0
                && chapterMaskSatisfied)
                return true;
            else
                return false;
        }
    }

    bool EvaluateDialogChoice(const Choice& choice) const
    {
        return std::visit(overloaded{
                [&](const EventFlagChoice& c){
                    return GetEventState(c.mEventPointer) == c.mExpectedValue;
                },
                [&](const ComplexEventChoice& c){
                    return EvaluateComplexChoice(c);
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

    std::uint8_t GetComplexEventState(unsigned eventPtr) const
    {
        if (mGameData != nullptr)
        {
            const auto res = mGameData->ReadComplexEvent(eventPtr);
            mLogger.Info() << __FUNCTION__ << "Read[" << +res << "]\n";
            return res;
        }
        else
        {
            return 0;
        }
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
        if (mGameData)
            mGameData->WriteComplexEvent(setFlag.mEventPointer, result);
        else
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

    std::vector<Container>& GetContainers(ZoneNumber zone)
    {
        assert(zone.mValue < 13);
        return mContainers[zone.mValue - 1];
    }

    Character mPartyLeader;
    Character mPartyFollower;
    GameData* mGameData;
    Party mParty;
    unsigned mContextValue;
    unsigned mSkillValue;
    unsigned mChapter;
    ZoneNumber mZone;
    std::vector<
        std::vector<Container>> mContainers;
    std::vector<GDSContainer> mGDSContainers;
    std::unordered_map<unsigned, bool> mEventState;
    const Logging::Logger& mLogger;
};

}
