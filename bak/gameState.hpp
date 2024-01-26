#pragma once

#include "bak/container.hpp"
#include "bak/constants.hpp"
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
        mCurrentMonster{},
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

    template <typename F, typename ...Args>
    bool Apply(F&& func, Args&&... args)
    {
        if (mGameData)
        {
            if constexpr (std::is_same_v<decltype(func(mGameData->GetFileBuffer(), args...)), bool>)
            {
                return std::invoke(func, mGameData->GetFileBuffer(), args...);
            }
            else
            {
                std::invoke(func, mGameData->GetFileBuffer(), args...);
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
            const auto hour = mGameData->mTime.GetTime().GetHour();
            return 0;
            //return static_cast<int>(hour < 18);
        }
        else
        {
            return 0; // daytime
            //return 1; // nighttime
        }
    }

    const WorldClock& GetWorldTime() const
    {
        if (mGameData)
        {
            return mGameData->mTime;
        }
        else
        {
            return mFakeWorldClock;
        }
    }

    WorldClock& GetWorldTime()
    {
        if (mGameData)
        {
            return mGameData->mTime;
        }
        else
        {
            return mFakeWorldClock;
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

    std::optional<unsigned> GetActor(unsigned actor) const
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
        else if (actor == 0xfe)
        {
            return GetPartyLeader().mValue + 1;
        }
        else if (actor == 0xfd)
        {
            // Use party follower index?
            // this logicc is not quite right
            return mDialogCharacterList[5] + 1;
        }
        else if (actor >= 0xf0)
        {
            //0xf0 - use the character list for diags to pick a character?
            assert(mDialogCharacterList[actor - 0xf0] != 0xff);
            return mDialogCharacterList[actor - 0xf0] + 1;
        }
        else if (actor == 0xc8)
        {
            return std::nullopt;
        }
        else
        {
            return actor;
        }
    }

    bool GetSpellActive(SpellIndex spell)
    {
        return true;
    }

    // prefer to use this function when getting best skill
    // as it will set the appropriate internal state.
    std::pair<CharIndex, unsigned> GetPartySkill(BAK::SkillType skill, bool best)
    {
        const auto [character, value] = GetParty().GetSkill(skill, best);
        mSkillCheckedCharacter = character;
        mSkillValue = value;
        return std::make_pair(character, value);
    }

    void SetCharacterTextVariables()
    {
        mEndOfDialogState = 0x0;

        if (GetParty().GetNumCharacters() > 0)
        {
            SetDefaultDialogTextVariables();
        }
    }

    void SetDefaultDialogTextVariables()
    {
        for (auto& c : mDialogCharacterList)
        {
            c = 0xff;
        }
        // index 4 gets party leader
        SetDialogTextVariable(4, 7);
        // Select random characters for indices 5, 3, and 0
        SetDialogTextVariable(5, 0xF);
        SetDialogTextVariable(3, 0xE);
        SetDialogTextVariable(0, 0x1f);
    }

    void SetDialogTextVariable(unsigned index, unsigned attribute)
    {
        mLogger.Debug() << __FUNCTION__ << "(" << index << ", " << attribute << ")\n";
        assert(attribute > 0);
        switch (attribute - 1)
        {
        case 0: [[fallthrough]];
        case 1: [[fallthrough]];
        case 2: [[fallthrough]];
        case 3: [[fallthrough]];
        case 4: [[fallthrough]];
        case 5:
            mDialogCharacterList[index] = attribute;
            mTextVariableStore.SetTextVariable(index, GetParty().GetCharacter(CharIndex{attribute}).GetName());
            break;
        case 6:
            mDialogCharacterList[index] = GetPartyLeader().mValue;
            mTextVariableStore.SetTextVariable(index, GetParty().GetCharacter(GetPartyLeader()).GetName());
            break;
        case 10:
            mDialogCharacterList[index] = mActiveCharacter.mValue;
            mTextVariableStore.SetTextVariable(index, GetParty().GetCharacter(mActiveCharacter).GetName());
            break;
        case 11:
            mDialogCharacterList[index] = mSkillCheckedCharacter.mValue;
            mTextVariableStore.SetTextVariable(index, GetParty().GetCharacter(mSkillCheckedCharacter).GetName());
            break;
        case 12: [[fallthrough]];
        case 13: [[fallthrough]];
        case 14: [[fallthrough]];
        case 15: [[fallthrough]];
        case 30:
            SelectRandomActiveCharacter(index, attribute);
            break;
        case 7: [[fallthrough]];
        case 8: [[fallthrough]];
        case 23: [[fallthrough]];
        case 24: [[fallthrough]];
        case 25:
            break; // Do nothing..?
        default:
             break;
        }
        mLogger.Debug() << __FUNCTION__ << " chose: " << +mDialogCharacterList[index] << "\n";
    }

    CharIndex GetPartyLeader() const
    {
        static constexpr std::array<CharIndex, 9> partyLeaderPerChapter = {Locklear, James, James, Gorath, James, Owyn, James, Owyn, Pug};
        if (GetParty().FindActiveCharacter(Pug))
        {
            return Pug;
        }
        else
        {
            return partyLeaderPerChapter[GetChapter().mValue - 1];
        }
    }

    void SelectRandomActiveCharacter(unsigned index, unsigned attribute)
    {
        mLogger.Debug() << __FUNCTION__ << "(" << index << ", " << attribute << ")\n";
        bool foundValidCharacter = false;
        unsigned limit = 0x1f8;
        // is this right...? or is it the opposite of the party leader that we should choose by default?
        auto chosenChar = GetPartyLeader();
        
        auto CharacterAlreadyExists = [&](auto character){
            for (unsigned i = 0; i < index; i++)
            {
                if (mDialogCharacterList[i] == character.mValue) return true;
            }
            return false;
        };

        while (!foundValidCharacter && limit != 0)
        {
            limit--;
            auto randomChar = ActiveCharIndex{GetRandomNumber(0, GetParty().GetNumCharacters() - 1)};
            chosenChar = GetParty().GetCharacter(randomChar).mCharacterIndex;
            if (CharacterAlreadyExists(chosenChar))
            {
                continue;
            }

            switch (attribute)
            {
            case 14: // Magicians
                if (chosenChar == Owyn
                    || chosenChar == Pug
                    || chosenChar == Patrus)
                {
                    foundValidCharacter = true;
                }
                break;
            case 15: // Swordsmen
                if (chosenChar == Locklear
                    || chosenChar == Gorath
                    || chosenChar == James)
                {
                    foundValidCharacter = true;
                }
                break;
            case 16: // Third-ary character?
                if (chosenChar == Gorath
                    || chosenChar == Patrus)
                {
                    foundValidCharacter = true;
                }
                break;
            case 31:
                if (chosenChar != GetPartyLeader())
                {
                    foundValidCharacter = true;
                }
                break;
            default:
                foundValidCharacter = true;
                break;
            }
        }

        mTextVariableStore.SetTextVariable(index, GetParty().GetCharacter(chosenChar).GetName());
        mDialogCharacterList[index] = chosenChar.mValue;
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
                if (set.mWhat == 0x7 || (set.mWhat >= 0xb && set.mWhat <= 0xf))
                {
                    SetDialogTextVariable(set.mWhich, set.mWhat);
                }
                else if (set.mWhat == 0x11)
                {
                    const auto& monsters = MonsterNames::Get();
                    mTextVariableStore.SetTextVariable(
                        set.mWhich,
                        mCurrentMonster ? monsters.GetMonsterName(*mCurrentMonster) : "No Monster Specified");
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
                    // if in a tavern use tavernkeeper for the shop
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
                auto amount = skill.mMax;
                if (skill.mMin != skill.mMax)
                {
                    amount = skill.mMin + (GetRandomNumber(0, 0xfff) % (skill.mMax - skill.mMin));
                }

                if (skill.mWho <= 1)
                {
                    mLogger.Debug() << "Gaining skill: " << skill << " - with amount: " << amount << " for all chars\n";
                    GetParty().ImproveSkillForAll(
                            skill.mSkill,
                            SkillChange::Direct,
                            amount);
                }
                // there are two character index arrays in the code, starting at 0x4df0 and 0x4df2
                // The dala one specifically uses query choice dialog to select the character, and 
                // places this in 4df2[0], i.e. 4df0[2], so the skill.mWho is effecting -= 2
                // Since skill.mWho <= 1 doesn't use the array anyway, we always use the array
                // that starts at 0x4df2
                else
                {
                    mLogger.Debug() << "Gaining skill: " << skill << " - with amount: " << amount << " for "
                        << +mDialogCharacterList[skill.mWho - 2] << "\n";
                    auto characterToImproveSkill = mDialogCharacterList[skill.mWho - 2];
                    GetParty().GetCharacter(CharIndex{characterToImproveSkill})
                        .ImproveSkill(skill.mSkill, SkillChange::Direct, amount);
                }
            },
            [&](const BAK::GainCondition& cond)
            {
                auto amount = cond.mMax;
                if (cond.mMin != cond.mMax)
                {
                    amount = cond.mMin + (GetRandomNumber(0, 0xfff) % (cond.mMax - cond.mMin));
                }
                if (cond.mWho > 1)
                {
                    auto characterToAffect = CharIndex{mDialogCharacterList[cond.mWho - 2]};
                    mLogger.Debug() << "Gaining condition : " << cond << " - with amount: " << amount << " for "
                        << characterToAffect << "\n";
                    GetParty().GetCharacter(characterToAffect).GetConditions()
                        .IncreaseCondition(cond.mCondition, amount);
                }
                else
                {
                    mLogger.Debug() << "Gaining condition : " << cond << " - with amount: " << amount << " for all\n";
                    GetParty().ForEachActiveCharacter(
                        [&](auto& character){
                            character.GetConditions().IncreaseCondition(
                                cond.mCondition, amount);
                            return false;
                        });
                }
            },
            [&](const BAK::SetTimeExpiringState& state)
            {
                mLogger.Debug() << "Setting time expiring state: " << state << "\n";
                SetEventValue(state.mEventPtr, 1);
                if (mGameData)
                    mGameData->SetTimeExpiringState(4, state.mEventPtr, 0x40, state.mTimeToExpire);
            },
            [&](const BAK::ElapseTime& elapse)
            {
                mLogger.Debug() << "Elapsing time: " << elapse << "\n";
                ElapseTime(elapse.mTime);
            },
            [&](const BAK::SetTimeExpiringState2& state)
            {
                mLogger.Debug() << "Setting time expiring state2: " << state << "\n";
                if (mGameData)
                    mGameData->SetTimeExpiringState(state.mNumber, state.mEventPtr, state.mFlag, state.mTimeToExpire);
            },
            [&](const BAK::SetEndOfDialogState& state)
            {
                mLogger.Debug() << "Setting end of dialog state: " << state << "\n";
                mEndOfDialogState = state.mState;
            },
            [&](const BAK::LoadSkillValue& load)
            {
                mLogger.Debug() << "Loading skill value: " << load << "\n";
                GetPartySkill(load.mSkill, load.mTarget == 1);
            },
            [&](const BAK::LearnSpell& learnSpell)
            {
                mLogger.Debug() << "Learning Spell: " << learnSpell << "\n";
                mParty.GetCharacter(CharIndex{mDialogCharacterList[learnSpell.mWho]})
                    .GetSpells().SetSpell(learnSpell.mWhichSpell);
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

    // in cpp to break dep with bak/camp.hpp
    void ElapseTime(Time time);

    bool EvaluateComplexChoice(const ComplexEventChoice& choice) const
    {
        // RunDialog addr: 23d1
        const auto state = GetEventState(choice.mEventPointer);

        // Probably want to put this logic somewhere else...
        // if eventPtr % 10 != 0
        const auto [byteOffset, bitOffset] = State::CalculateComplexEventOffset(choice.mEventPointer);
        mLogger.Debug() << __FUNCTION__ << " : " << choice 
            << " S: [" << std::hex << +state << std::dec << "] - byteOff: " 
            << + byteOffset << " bitOff: " << +bitOffset << "\n";

        if (mGameData && bitOffset != 0)
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
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (((state ^ choice.mXorMask) & choice.mExpected) != 0
                && chapterMaskSatisfied)
            {
                return true;
            }
            else
            {
                return false;
            }
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
            [&](const CastSpellChoice& c)
            {
                //if (c.mRequiredSpell == 5) // scent of sarig
                //if (c.mRequiredSpell == 3) // the unseen
                return true;
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
    CharIndex mSkillCheckedCharacter;

    // 0 - defaults to someone other than party leader
    // 1 - will be selected so as not to equal 0
    // 2 - will be selected so as not to be either 0 or 1
    // 3 - party magician
    // 4 - party leader
    // 5 - party warrior
    std::array<std::uint8_t, 6> mDialogCharacterList;

    GameData* mGameData;
    Party mParty;
    unsigned mContextValue;
    unsigned mShopType;
    Royals mItemValue;
    unsigned mSkillValue;
    std::optional<InventoryItem> mSelectedItem;
    std::optional<MonsterIndex> mCurrentMonster;
    Chapter mChapter;
    ZoneNumber mZone;
    std::int16_t mEndOfDialogState;
    std::vector<
        std::vector<GenericContainer>> mContainers;
    std::vector<GenericContainer> mGDSContainers;
    std::vector<GenericContainer> mCombatContainers;
    TextVariableStore mTextVariableStore;
    WorldClock mFakeWorldClock{{0}, {0}};
    const Logging::Logger& mLogger;
};

}
