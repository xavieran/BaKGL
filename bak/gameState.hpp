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

    unsigned GetActor(unsigned actor) const
    {
        if (actor == 0xff)
        {
            if (!mDialogCharacter)
            {
                const auto& character = GetParty().GetCharacter(ActiveCharIndex{0});
                return character.mCharacterIndex.mValue + 1;
            }
            else
            {
                const auto& character = GetParty().GetCharacter(*mDialogCharacter);
                return character.mCharacterIndex.mValue + 1;
            }
        }
        else if (actor == 0xf0)
        {
            const auto& character = GetParty().GetCharacter(ActiveCharIndex{0});
            return character.mCharacterIndex.mValue + 1;
        }
        // WRONG! f4 same as text variable 4...
        else if (actor > 0xf0)
        {
            return actor & 0xf;
        }
        else if (actor == 0xc8)
        {
            const auto& character = GetParty().GetCharacter(
                GetParty().NextActiveCharacter(
                    GetParty().NextActiveCharacter(
                        ActiveCharIndex{0})));
            return character.mCharacterIndex.mValue + 1;
        }
        else
        {
            return actor;
        }
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
        // RunDialog addr: 23d1
        const auto state = GetEventState(choice.mEventPointer);
        mLogger.Debug() << __FUNCTION__ << "Choice: " << choice 
            << " S: [" << std::hex << +state << std::dec << "]\n";

        // Probably want to put this logic somewhere else...
        // if eventPtr % 10 != 0
        if (mGameData 
            && std::get<1>(mGameData->CalculateComplexEventOffset(choice.mEventPointer))
                != 0)
        {
            return (state >= choice.mXorMask) && (state <= choice.mMustEqualExpected);
        }

        // Need to double check this...
        //const auto chapterFlag = GetChapter() == 9
        //    ? 0x80
        //    : 1 << (GetChapter() - 1);
        //const auto chapterMaskSatisfied
        //    = (chapterFlag & choice.mChapterMask) != 0;
        const auto chapterMaskSatisfied = true;

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
                    return GetEventStateBool(c.mEventPointer) == c.mExpectedValue;
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

    unsigned GetEventState(unsigned eventPtr) const
    {
        if (mGameData != nullptr)
            return mGameData->ReadEvent(eventPtr);
        else
            return 0;
    }

    unsigned GetEventStateBool(unsigned eventPtr) const
    {
        return (GetEventState(eventPtr) & 0x1) == 1;
    }

    bool CheckInhibited(const ConversationChoice& choice)
    {
        if (mGameData != nullptr)
        {
            return mGameData->CheckConversationOptionInhibited(
                choice.mEventPointer);
        }
        return false;
    }

    bool CheckDiscussed(const ConversationChoice& choice)
    {
        if (mGameData != nullptr)
        {
            return mGameData->ReadConversationItemClicked(
                choice.mEventPointer);
        }
        return false;
    }

    void MarkDiscussed(const ConversationChoice& choice)
    {
        if (mGameData)
            mGameData->SetConversationItemClicked(choice.mEventPointer);
    }

    void SetEventValue(unsigned eventPtr, unsigned value)
    {
        if (mGameData)
            mGameData->SetEventFlag(eventPtr, value);
    }

    void SetEventState(const SetFlag& setFlag)
    {
        if (mGameData)
            mGameData->SetEventDialogAction(setFlag);
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

    std::optional<CharIndex> mDialogCharacter;

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
    const Logging::Logger& mLogger;
};

}
