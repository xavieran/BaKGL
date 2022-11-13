#pragma once

#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogChoice.hpp"
#include "bak/gameData.hpp"
#include "bak/money.hpp"
#include "bak/save.hpp"
#include "bak/textVariableStore.hpp"
#include "bak/types.hpp"

#include "com/random.hpp"
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
        mGameData{gameData},
        mParty{
            Royals{1000},
            Inventory{20},
            std::vector<Character>{
                Character{
                    0,
                    "None",
                    Skills{},
                    {},
                    {},
                    {},
                    Conditions{},
                    Inventory{5}}},
            std::vector<CharIndex>{CharIndex{0}}},
        mContextValue{0},
        mShopType{0},
        mItemValue{0},
        mSkillValue{0},
        mSelectedItem{},
        mChapter{7},
        mZone{1},
        mEndOfDialogState{0},
        mContainers{},
        mGDSContainers{},
        mCombatContainers{},
        mTextVariableStore{},
        mLogger{Logging::LogState::GetLogger("BAK::GameState")}
    {
        if (mGameData != nullptr)
        {
            LoadGameData(mGameData);
        }
    }

    void LoadGameData(GameData* gameData)
    {
        ASSERT(gameData);
        mGameData = gameData;
        mGDSContainers.clear();
        mCombatContainers.clear();
        mContainers.clear();
        for (unsigned i = 0; i < 12; i++)
        {
            mContainers.emplace_back(mGameData->LoadContainers(i + 1));
        }
        mGDSContainers = mGameData->LoadShops();
        mCombatContainers = mGameData->LoadCombatInventories();
        mZone = ZoneNumber{mGameData->mLocation.mZone};
    }

    const Party& GetParty() const
    {
        if (mGameData)
        {
            return mGameData->mParty;
        }
        return mParty;
    }

    Party& GetParty()
    {
        if (mGameData)
        {
            return mGameData->mParty;
        }
        return mParty;
    }

    std::int16_t GetEndOfDialogState() const
    {
        return mEndOfDialogState;
    }

    GameData& GetGameData()
    {
        ASSERT(mGameData);
        return *mGameData;
    }

    void SetActiveCharacter(ActiveCharIndex character)
    {
        mTextVariableStore.SetActiveCharacter(
            GetParty().GetCharacter(character).mName);
    }

    void SetActiveCharacter(CharIndex character)
    {
        mTextVariableStore.SetActiveCharacter(
            GetParty().GetCharacter(character).mName);
    }
    
    const TextVariableStore& GetTextVariableStore() const { return mTextVariableStore; }
    TextVariableStore& GetTextVariableStore() { return mTextVariableStore; }

    void SetChapter(Chapter chapter)
    {
        if (mGameData)
            mGameData->mChapter = chapter;
        else
            mChapter = chapter;
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
        mZone = ZoneNumber{loc.mZone};
        if (mGameData)
        {
            mGameData->mLocation = loc;
        }
    }

    void SetLocation(BAK::GamePositionAndHeading posAndHeading)
    {
        if (mGameData)
        {
            const auto loc = Location{
                mZone.mValue,
                GetTile(posAndHeading.mPosition),
                posAndHeading};
            mGameData->mLocation = loc;
        }
    }

    BAK::GamePositionAndHeading GetLocation() const
    {
        if (mGameData)
            return mGameData->mLocation.mLocation;

        return BAK::GamePositionAndHeading{ glm::uvec2{10 * 64000, 15 * 64000}, 0 };
    }

    auto GetZone() const
    {
        if (mGameData)
        {
            return ZoneNumber{mGameData->mLocation.mZone};
        }
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

    void SetShopType(unsigned shopType)
    {
        mShopType = shopType;
    }

    auto GetShopType() const
    {
        ASSERT(mShopType != 0);
        return mShopType;
    }

    IContainer* GetContainerForGDSScene(BAK::HotspotRef ref)
    {
        for (auto& shop : mGDSContainers)
        {
            if (shop.GetHeader().GetHotspotRef() == ref)
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
        else if (actor == 0xf0 || actor == 0xf4)
        {
            const auto& character = GetParty().GetCharacter(ActiveCharIndex{0});
            return character.mCharacterIndex.mValue + 1;
        }
        else if (actor == 0xf3 || actor == 0xf1)
        {
            const auto& character = GetParty().GetCharacter(ActiveCharIndex{1});
            return character.mCharacterIndex.mValue + 1;
        }
        else if (actor == 0xf5)
        {
            const auto& character = GetParty().GetCharacter(ActiveCharIndex{2});
            return character.mCharacterIndex.mValue + 1;
        }
        else if (actor > 0xf5)
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

    bool GetSpellActive(unsigned spell)
    {
        return true;
    }

    void SetCharacterTextVariables()
    {
        mEndOfDialogState = 0x0;

        // FIXME: There is more to setting the characters than this...
        if (GetParty().GetNumCharacters() > 0)
        {
            mTextVariableStore.SetTextVariable(0, GetParty().GetCharacter(ActiveCharIndex{0}).GetName());
            mTextVariableStore.SetTextVariable(4, GetParty().GetCharacter(ActiveCharIndex{0}).GetName());
            mTextVariableStore.SetTextVariable(3, GetParty().GetCharacter(ActiveCharIndex{1}).GetName());
            if (GetParty().GetNumCharacters() > 2)
                mTextVariableStore.SetTextVariable(5, GetParty().GetCharacter(ActiveCharIndex{2}).GetName());
        }
    }

    void EvaluateAction(const DialogAction& action)
    {
        std::visit(overloaded{
            [&](const BAK::SetFlag& set)
            {
                mLogger.Debug() << "Setting flag of event: " << BAK::DialogAction{set} << "\n";
                SetEventState(set);
            },
            [&](const BAK::SetTextVariable& set)
            {
                mLogger.Debug() << "Setting text variable: " << BAK::DialogAction{set} << "\n";
                if (set.mWhat == 0x7 || set.mWhat == 0xc || set.mWhat == 0xf)
                {
                    mTextVariableStore.SetTextVariable(set.mWhich, GetParty().GetCharacter(ActiveCharIndex{0}).GetName());
                }
                else if (set.mWhat == 0xd || set.mWhat == 0xe)
                {
                    const auto character = GetRandomNumber(1, GetParty().GetNumCharacters() - 1);
                    mTextVariableStore.SetTextVariable(set.mWhich, GetParty().GetCharacter(ActiveCharIndex{character}).GetName());
                }
                else if (set.mWhat == 0x11)
                {
                    mTextVariableStore.SetTextVariable(
                        set.mWhich,
                        "Opponent");
                }
                else if (set.mWhat == 0x12)
                {
                    ASSERT(mSelectedItem);
                    mTextVariableStore.SetTextVariable(
                        set.mWhich,
                        mSelectedItem->GetObject().mName);
                }
                else if (set.mWhat == 0x13)
                {
                    mTextVariableStore.SetTextVariable(
                        set.mWhich,
                        ToShopDialogString(mItemValue));
                }
                else if (set.mWhat == 0x19)
                {
                    mTextVariableStore.SetTextVariable(
                        set.mWhich,
                        ToShopDialogString(mItemValue));
                }
                else if (set.mWhat == 0x1c)
                {
                    mTextVariableStore.SetTextVariable(set.mWhich, "shopkeeper");
                }
            },
            [&](const BAK::LoseItem& lose)
            {
                mLogger.Debug() << "Losing item: " << lose << "\n";
                auto& party = GetParty();
                party.RemoveItem(lose.mItemIndex, lose.mQuantity);
            },
            [&](const BAK::GiveItem& give)
            {
                mLogger.Debug() << "Giving item: " << give << "\n";
                auto& party = GetParty();
                party.GainItem(give.mCharacter, give.mItemIndex, give.mQuantity);
            },
            [&](const BAK::GainSkill& skill)
            {
                // FIXME: Implement if this skill only improves for one character
                // Refer to dala blessing (13 @ 0xfd8c)
                // who == 6 => apply gain to selected character (ref gamestate)
                // who == 2 => apply gain to selected character (ref gamestate)
                // who == 1 => apply gain to all characters
                // Ref label: RunDialog_GainSkillAction for actual code
                // If Skill == TotalHealth: 
                // mValue 1 and mValue 2 refer to the upper and lower bounds of change
                mLogger.Debug() << "Gaining skill: " << skill << "\n";
                GetParty().ImproveSkillForAll(
                        skill.mSkill,
                        SkillChange::Direct,
                        skill.mValue1);
            },
            [&](const BAK::GainCondition& cond)
            {
                mLogger.Debug() << "Gaining condition: " << cond << "\n";
                // FIXME: Implement if this condition only changes for one character
                auto& party = GetParty();
                for (const auto c : party.mActiveCharacters)
                {
                    party.mCharacters[c.mValue].mConditions.IncreaseCondition(
                        cond.mCondition, cond.mValue1);
                }
            },
            [&](const BAK::SetState& state)
            {
                mLogger.Debug() << "Setting state: " << state << "\n";
                SetEventValue(state.mEventPtr, 1);
            },
            [&](const BAK::SetEndOfDialogState& state)
            {
                mLogger.Debug() << "Setting end of dialog state: " << state << "\n";
                mEndOfDialogState = state.mState;
            },
            [&](const BAK::LoadSkillValue& load)
            {
                mLogger.Debug() << "Loading skill value: " << load << "\n";
                const auto [character, value] = GetParty().GetSkill(
                    load.mSkill,
                    load.mTarget == 1); // best or worst skill
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
            && (GetChapter().mValue >= choice.mExpectedValue
                && GetChapter().mValue <= choice.mExpectedValue2))
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::Context
            && mContextValue == choice.mExpectedValue)
        {
            return true;
        }
        else if (choice.mState == BAK::ActiveStateFlag::CantAfford
            && (GetMoney() > mItemValue) == (choice.mExpectedValue == 1))
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
        mLogger.Debug() << __FUNCTION__ << " : " << choice 
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
                [&](const NoChoice& c){
                    return true;
                },
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
                    return false; 
                },
            },
            choice);
    }

    unsigned GetEventState(unsigned eventPtr) const
    {
        if (mGameData != nullptr)
        {
            return mGameData->ReadEvent(eventPtr);
        }
        else
            return 0;
    }

    bool GetEventStateBool(unsigned eventPtr) const
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

    bool CheckLockSeen(unsigned lockIndex)
    {
        if (mGameData)
            return mGameData->CheckLockHasBeenSeen(lockIndex);
        return false;
    }

    void MarkLockSeen(unsigned lockIndex)
    {
        if (mGameData)
            mGameData->SetLockHasBeenSeen(lockIndex);
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

    void SetTempleSeen(unsigned temple)
    {
        if (mGameData)
            mGameData->SetTempleSeen(temple);
    }

    bool GetTempleSeen(unsigned temple) const
    {
        if (mGameData)
            return mGameData->ReadTempleSeen(temple);
        return true;
    }

    bool GetMoreThanOneTempleSeen() const
    {
        if (mGameData)
        {
            unsigned templesSeen = 0;
            for (unsigned i = 1; i < 13; i++)
            {
                templesSeen += GetTempleSeen(i);
            }
            return templesSeen > 1;
        }
        return true;
    }

    bool CheckEncounterActive(const Encounter::Encounter& encounter)
    {
         if (!mGameData) return true;
         return mGameData->CheckActive(encounter, mZone);
    }

    void SetPostDialogEventFlags(const Encounter::Encounter& encounter)
    {
        if (mGameData)
            GetGameData().SetPostDialogEventFlags(
                encounter,
                mZone);
    }

    void SetPostGDSEventFlags(const Encounter::Encounter& encounter)
    {
        if (mGameData)
            mGameData->SetPostGDSEventFlags(encounter);
    }

    void SetPostEnableOrDisableEventFlags(const Encounter::Encounter& encounter)
    {
        if (mGameData)
            mGameData->SetPostEnableOrDisableEventFlags(encounter, mZone);
    }



    void SetDialogContext(unsigned contextValue)
    {
        mContextValue = contextValue;
    }

    void SetItemValue(Royals value)
    {
        mItemValue = value;
    }

    void SetInventoryItem(const InventoryItem& item)
    {
        mSelectedItem = item;
    }

    void ClearUnseenImprovements(unsigned character)
    {
        if (mGameData)
        {
            mGameData->ClearUnseenImprovements(character);
        }
    }

    bool Save(const std::string& saveName)
    {
        if (mGameData)
        {
            BAK::Save(GetParty(), mGameData->GetFileBuffer());

            for (const auto& container : mGDSContainers)
                BAK::Save(container, mGameData->GetFileBuffer());

            for (const auto& container : mCombatContainers)
                BAK::Save(container, mGameData->GetFileBuffer());

            for (const auto& zoneContainers : mContainers)
                for (const auto& container : zoneContainers)
                    BAK::Save(container, mGameData->GetFileBuffer());

            mGameData->Save(saveName);
            return true;
        }
        return false;
    }

    std::vector<GenericContainer>& GetContainers(ZoneNumber zone)
    {
        ASSERT(zone.mValue < 13);
        return mContainers[zone.mValue - 1];
    }

    std::optional<CharIndex> mDialogCharacter;

    GameData* mGameData;
    Party mParty;
    unsigned mContextValue;
    unsigned mShopType;
    Royals mItemValue;
    unsigned mSkillValue;
    std::optional<InventoryItem> mSelectedItem;
    Chapter mChapter;
    ZoneNumber mZone;
    std::int16_t mEndOfDialogState;
    std::vector<
        std::vector<GenericContainer>> mContainers;
    std::vector<GenericContainer> mGDSContainers;
    std::vector<GenericContainer> mCombatContainers;
    TextVariableStore mTextVariableStore;
    const Logging::Logger& mLogger;
};

}
