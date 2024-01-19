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

#include "bak/state/dialog.hpp"
#include "bak/state/event.hpp"
#include "bak/state/encounter.hpp"
#include "bak/state/lock.hpp"
#include "bak/state/skill.hpp"
#include "bak/state/temple.hpp"

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
        mDialogCharacter{},
        mActiveCharacter{CharIndex{0}},
        mGameData{gameData},
        mParty{
            Royals{1000},
            Inventory{20},
            std::vector<Character>{
                Character{
                    0,
                    "None",
                    Skills{},
                    Spells{{}},
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
        mActiveCharacter = GetParty().GetCharacter(character).mCharacterIndex;
        mTextVariableStore.SetActiveCharacter(
            GetParty().GetCharacter(character).mName);
    }

    void SetActiveCharacter(CharIndex character)
    {
        mActiveCharacter = character;
        mTextVariableStore.SetActiveCharacter(
            GetParty().GetCharacter(character).mName);
    }
    
    const TextVariableStore& GetTextVariableStore() const { return mTextVariableStore; }
    TextVariableStore& GetTextVariableStore() { return mTextVariableStore; }

    template <typename F>
    bool Apply(F&& func)
    {
        if (mGameData)
        {
            if constexpr (std::is_same_v<decltype(func(mGameData->GetFileBuffer())), bool>)
            {
                return std::invoke(func, mGameData->GetFileBuffer());
            }
            else
            {
                std::invoke(func, mGameData->GetFileBuffer());
            }
        }

        return false;
    }

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

    WorldClock GetWorldTime() const
    {
        if (mGameData)
        {
            return mGameData->mTime;
        }
        else
        {
            return WorldClock{{0}, {0}};
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
                else if (set.mWhat == 0xb)
                {
                    mTextVariableStore.SetTextVariable(
                        set.mWhich,
                        GetParty().GetCharacter(mActiveCharacter).GetName());
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
                if (skill.mWho <= 1)
                {
                    GetParty().ImproveSkillForAll(
                            skill.mSkill,
                            SkillChange::Direct,
                            skill.mValue1);
                }
                // 3, 4 -> Owyn & Gorath when talking to Calin
                // 3, 4, -> Locklear and James when talking to Martin
                // 5 -> Owyn when talking to Cullich
                // 6 -> Party leader
                //  0  1  2  3  4  5  6  7 
                // 00 00 00 02 01 02 00 01 00 Chapter 1
                // 00 00 02 FF FF 02 04 01 00 Chapter 2
                // 00 00 02 FF FF 02 04 01 00 Chapter 3
                // 00 00 02 FF FF 02 01 01 00 Chapter 4
                // 00 00 05 FF FF 05 04 00 00 Chapter 5
                // 00 00 02 FF FF 02 00 01 00 Chapter 6
                // use this to figure out who is who...
                // This is generated semi-randomly from the active characters list
                else if (skill.mWho == 6) // 1,2,3,4,5,6, (condition -- 1,2,6,7)
                {
                    GetParty().GetCharacter(ActiveCharIndex{0})
                        .ImproveSkill(skill.mSkill, SkillChange::Direct, skill.mValue1);
                }
                else if (skill.mWho == 2)
                {
                    GetParty().GetCharacter(ActiveCharIndex{0})
                        .ImproveSkill(skill.mSkill, SkillChange::Direct, skill.mValue1);
                }
            },
            [&](const BAK::GainCondition& cond)
            {
                mLogger.Debug() << "Gaining condition: " << cond << "\n";
                // FIXME: Implement if this condition only changes for one character
                GetParty().ForEachActiveCharacter(
                    [&](auto& character){
                        character.GetConditions().IncreaseCondition(
                            cond.mCondition, cond.mValue1);
                        return true;
                    });
            },
            [&](const BAK::SetTimeExpiringState& state)
            {
                mLogger.Debug() << "Setting time expiring state: " << state << "\n";
                SetEventValue(state.mEventPtr, 1);
                if (mGameData)
                    mGameData->SetTimeExpiringState(4, state.mEventPtr, 0x40, state.mTimeToExpire);
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
            [&](const BAK::LearnSpell& learnSpell)
            {
                // This is always 5 - which must mean the party magician
                mLogger.Debug() << "Learning Spell: " << learnSpell << "\n";
                assert(learnSpell.mWho == 5);
                mParty.ForEachActiveCharacter(
                    [&](auto& character){
                        if (character.IsSpellcaster())
                        {
                            character.GetSpells().SetSpell(learnSpell.mWhichSpell);
                            return true;
                        }
                        return false;
                    });
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
        else if (choice.mState == BAK::ActiveStateFlag::GameTime
            && 1 == choice.mExpectedValue)//GetTime() == choice.mExpectedValue)
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

    void ElapseTime(Time time)
    {
        GetParty().ForEachActiveCharacter([&](auto& character){
            // Heal when camping...
            if (character.CanHeal(false))
            {
                character.ImproveSkill(
                    SkillType::TotalHealth,
                    static_cast<SkillChange>(100),
                    1 << 8);
            }
            return false;
        });
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
            && std::get<1>(State::CalculateComplexEventOffset(choice.mEventPointer))
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
            [&](const CustomStateChoice& c)
            {
                switch (c.mScenario)
                {
                case Scenario::Plagued:
                    return CheckCustomStateScenarioPlagued();
                case Scenario::AllPartyArmorIsGoodCondition:
                    return CheckCustomStateScenarioAllPartyArmorIsGoodCondition();
                default:
                    return false;
                }
            },
            [&](const auto& c){
                return false; 
            },
        },
        choice);
    }

    unsigned GetEventState(unsigned eventPtr) const
    {
        if (eventPtr == static_cast<unsigned>(ActiveStateFlag::Chapter))
        {
            return GetChapter().mValue;
        }
        if (mGameData != nullptr)
        {
            return State::ReadEvent(mGameData->GetFileBuffer(), eventPtr);
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
            return State::CheckConversationOptionInhibited(
                mGameData->GetFileBuffer(),
                choice.mEventPointer);
        }
        return false;
    }

    bool CheckDiscussed(const ConversationChoice& choice)
    {
        if (mGameData != nullptr)
        {
            return State::ReadConversationItemClicked(
                mGameData->GetFileBuffer(),
                choice.mEventPointer);
        }
        return false;
    }

    void MarkDiscussed(const ConversationChoice& choice)
    {
        if (mGameData)
            State::SetConversationItemClicked(
                mGameData->GetFileBuffer(),
                choice.mEventPointer);
    }

    bool CheckLockSeen(unsigned lockIndex)
    {
        if (mGameData)
            return State::CheckLockHasBeenSeen(mGameData->GetFileBuffer(), lockIndex);
        return false;
    }

    void MarkLockSeen(unsigned lockIndex)
    {
        if (mGameData)
            State::SetLockHasBeenSeen(mGameData->GetFileBuffer(), lockIndex);
    }

    void SetEventValue(unsigned eventPtr, unsigned value)
    {
        if (mGameData)
            State::SetEventFlag(mGameData->GetFileBuffer(), eventPtr, value);
    }

    void SetEventState(const SetFlag& setFlag)
    {
        if (mGameData)
            State::SetEventDialogAction(mGameData->GetFileBuffer(), setFlag);
    }

    void SetTempleSeen(unsigned temple)
    {
        if (mGameData)
            State::SetTempleSeen(mGameData->GetFileBuffer(), temple);
    }

    bool GetTempleSeen(unsigned temple) const
    {
        if (mGameData)
            return State::ReadTempleSeen(mGameData->GetFileBuffer(), temple);
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
         return State::CheckActive(mGameData->GetFileBuffer(), encounter, mZone);
    }

    void SetPostDialogEventFlags(const Encounter::Encounter& encounter)
    {
        if (mGameData)
            State::SetPostDialogEventFlags(
                mGameData->GetFileBuffer(),
                encounter,
                mZone);
    }

    void SetPostGDSEventFlags(const Encounter::Encounter& encounter)
    {
        if (mGameData)
            State::SetPostGDSEventFlags(mGameData->GetFileBuffer(), encounter);
    }

    void SetPostEnableOrDisableEventFlags(const Encounter::Encounter& encounter)
    {
        if (mGameData)
            State::SetPostEnableOrDisableEventFlags(mGameData->GetFileBuffer(), encounter, mZone);
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
            State::ClearUnseenImprovements(mGameData->GetFileBuffer(), character);
        }
    }

    bool Save(const SaveFile& saveFile)
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

            mGameData->Save(saveFile);
            return true;
        }
        return false;
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

            mGameData->Save(saveName, saveName);
            return true;
        }
        return false;
    }

    std::vector<GenericContainer>& GetContainers(ZoneNumber zone)
    {
        ASSERT(zone.mValue < 13);
        return mContainers[zone.mValue - 1];
    }

    bool CheckCustomStateScenarioPlagued() const
    {
        bool foundPlagued = false;
        GetParty().ForEachActiveCharacter(
            [&](const auto& character){
                if (character.GetConditions().GetCondition(BAK::Condition::Plagued).Get() > 0)
                {
                    foundPlagued = true;
                    return false;
                }
                return true;
            });

        return foundPlagued;
    }

    bool CheckCustomStateScenarioAllPartyArmorIsGoodCondition() const
    {
        bool foundRepairableArmor = false;
        GetParty().ForEachActiveCharacter(
            [&](const auto& character){
                const auto& items = character.GetInventory().GetItems();
                for (const auto& item : items)
                {
                    if (item.IsItemType(BAK::ItemType::Armor) && item.IsRepairableByShop())
                    {
                        foundRepairableArmor = true;
                        return false;
                    }
                }
                return true;
            });

        return !foundRepairableArmor;
    }

    std::optional<CharIndex> mDialogCharacter;
    CharIndex mActiveCharacter;

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
