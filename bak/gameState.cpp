#include "bak/gameState.hpp"

#include "bak/spells.hpp"
#include "bak/time.hpp"

namespace BAK {

GameState::GameState()
:
    GameState{nullptr}
{}

GameState::GameState(
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
    mContextValue_7530{0},
    mShopType{0},
    mItemValue{0},
    mSkillValue{0},
    mSelectedItem{},
    mCurrentMonster{},
    mChapter{1},
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

void GameState::LoadGameData(GameData* gameData)
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
    mTimeExpiringState = mGameData->LoadTimeExpiringState();
    mSpellState = mGameData->LoadSpells();
    mZone = ZoneNumber{mGameData->mLocation.mZone};
    mChapter = mGameData->mChapter;
}

const Party& GameState::GetParty() const
{
    if (mGameData)
    {
        return mGameData->mParty;
    }
    return mParty;
}

Party& GameState::GetParty()
{
    if (mGameData)
    {
        return mGameData->mParty;
    }
    return mParty;
}

std::int16_t GameState::GetEndOfDialogState() const
{
    return mEndOfDialogState;
}

GameData& GameState::GetGameData()
{
    ASSERT(mGameData);
    return *mGameData;
}

void GameState::SetActiveCharacter(ActiveCharIndex character)
{
    mActiveCharacter = GetParty().GetCharacter(character).mCharacterIndex;
    mTextVariableStore.SetActiveCharacter(
        GetParty().GetCharacter(character).mName);
}

void GameState::SetActiveCharacter(CharIndex character)
{
    mActiveCharacter = character;
    mTextVariableStore.SetActiveCharacter(
        GetParty().GetCharacter(character).mName);
}

const TextVariableStore& GameState::GetTextVariableStore() const { return mTextVariableStore; }
TextVariableStore& GameState::GetTextVariableStore() { return mTextVariableStore; }

void GameState::SetChapter(Chapter chapter)
{
    mChapter = chapter;
}

Chapter GameState::GetChapter() const
{
    return mChapter;
}

Royals GameState::GetMoney() const
{
    if (mGameData)
        return mGameData->mParty.GetGold();

    return Royals{1000};
}

void GameState::SetLocation(BAK::Location loc)
{
    mLogger.Debug() << "SetLocation to: " << loc << "\n";
    mZone = ZoneNumber{loc.mZone};
    if (mGameData)
    {
        mGameData->mLocation = loc;
    }
}

void GameState::SetLocation(BAK::GamePositionAndHeading posAndHeading)
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

BAK::GamePositionAndHeading GameState::GetLocation() const
{
    if (mGameData)
        return mGameData->mLocation.mLocation;

    return BAK::GamePositionAndHeading{ glm::uvec2{10 * 64000, 15 * 64000}, 0 };
}

ZoneNumber GameState::GetZone() const
{
    return mZone;
}

const WorldClock& GameState::GetWorldTime() const
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

WorldClock& GameState::GetWorldTime()
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

void GameState::SetShopType(unsigned shopType)
{
    mShopType = shopType;
}

auto GameState::GetShopType() const
{
    ASSERT(mShopType != 0);
    return mShopType;
}

IContainer* GameState::GetContainerForGDSScene(BAK::HotspotRef ref)
{
    for (auto& shop : mGDSContainers)
    {
        if (shop.GetHeader().GetHotspotRef() == ref)
            return &shop;
    }
    return nullptr;
}

std::optional<unsigned> GameState::GetActor(unsigned actor) const
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

bool GameState::GetSpellActive(StaticSpells spell) const
{
    return mSpellState.SpellActive(spell);
}

// prefer to use this function when getting best skill
// as it will set the appropriate internal state.
std::pair<CharIndex, unsigned> GameState::GetPartySkill(BAK::SkillType skill, bool best)
{
    const auto [character, value] = GetParty().GetSkill(skill, best);
    mSkillCheckedCharacter = character;
    mSkillValue = value;
    return std::make_pair(character, value);
}

void GameState::SetCharacterTextVariables()
{
    mEndOfDialogState = 0x0;

    if (GetParty().GetNumCharacters() > 0)
    {
        SetDefaultDialogTextVariables();
    }
}

void GameState::SetDefaultDialogTextVariables()
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

void GameState::SetDialogTextVariable(unsigned index, unsigned attribute)
{
    mLogger.Debug() << __FUNCTION__ << "(" << index << ", " << attribute << ")\n";
    assert(attribute > 0);
    // remember we always switch on attribute - 1...
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
    case 16:
        mTextVariableStore.SetTextVariable(
            index,
            mCurrentMonster ? MonsterNames::Get().GetMonsterName(*mCurrentMonster) : "No Monster Specified");
        break;
    case 17:
        ASSERT(mSelectedItem);
        mTextVariableStore.SetTextVariable(
            index,
            mSelectedItem->GetObject().mName);
        break;
    case 18:
        mTextVariableStore.SetTextVariable(
            index,
            ToShopDialogString(mItemValue));
        break;
    case 19:
        mTextVariableStore.SetTextVariable(
            index,
            ToShopDialogString(GetParty().GetGold()));
        break;
    case 20:
        // mContextVar_valueOfItem - but used
        // in the restoratives dialog to indicate the
        // number of health points left...
        mTextVariableStore.SetTextVariable(
            index,
            "");
            // ToString(mContextVar_charCurrentHealthPoint)
        break;
    case 21:
        mTextVariableStore.SetTextVariable(
            index,
            ""); // ToString(mContextVar_753f)
    case 22:
        // unused??
        // ToString(mContext_whichSkillIncreased)
        break;
    case 26:
        // mTextVariableStore.SetTextVariable(index,
        //    ToString(BAK::SkillType(mContextVar_753f)));
        break;
    case 27:
        // or tavernkeeper if in an inn... which is determined by
        // inn cost being >= 0
        mTextVariableStore.SetTextVariable(index, "shopkeeper");
        break;
    case 28:
        // the skill that increased, either the party's or the
        // selected character
        // mContext_whichSkillIncreased
        mTextVariableStore.SetTextVariable(index, "skill");
        break;
    case 9:
    case 29:
        // character index is stored in checkSkillValue
        //mDialogCharacterList[index] =checkedSkillValue;
        mTextVariableStore.SetTextVariable(
            index,
            ""); // character at checked skill value
        break;
    case 31:
        // actor name
        break;
    default:
         break;
    }
    mLogger.Debug() << __FUNCTION__ << " chose: " << +mDialogCharacterList[index] << "\n";
}

CharIndex GameState::GetPartyLeader() const
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

void GameState::SelectRandomActiveCharacter(unsigned index, unsigned attribute)
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

void GameState::EvaluateAction(const DialogAction& action)
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
        [&](const BAK::SetAddResetState& state)
        {
            mLogger.Debug() << "Setting time expiring state: " << state << "\n";
            SetEventValue(state.mEventPtr, 1);
            AddTimeExpiringState(mTimeExpiringState, ExpiringStateType::ResetState, state.mEventPtr, 0x40, state.mTimeToExpire);
        },
        [&](const BAK::ElapseTime& elapse)
        {
            mLogger.Debug() << "Elapsing time: " << elapse << "\n";
            ElapseTime(elapse.mTime);
        },
        [&](const BAK::SetTimeExpiringState& state)
        {
            mLogger.Debug() << "Setting time expiring state2: " << state << "\n";
            AddTimeExpiringState(
                mTimeExpiringState,
                state.mType,
                state.mEventPtr,
                state.mFlags,
                state.mTimeToExpire);
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

bool GameState::EvaluateGameStateChoice(const GameStateChoice& choice) const
{
    if (choice.mState == BAK::ActiveStateFlag::Chapter)
    {
        return GetChapter().mValue >= choice.mExpectedValue
            && GetChapter().mValue <= choice.mExpectedValue2;
    }
    else if (choice.mState == BAK::ActiveStateFlag::Context)
    {
        return mContextValue_7530 == choice.mExpectedValue;
    }
    else if (choice.mState == BAK::ActiveStateFlag::CantAfford)
    {
        return (GetMoney() > mItemValue) == (choice.mExpectedValue == 1);
    }
    else if (choice.mState == BAK::ActiveStateFlag::SkillCheck)
    {
        return mSkillValue >= choice.mExpectedValue;
    }
    else if (choice.mState == BAK::ActiveStateFlag::Money)
    {
        return GetMoney().mValue > GetRoyals(Sovereigns{static_cast<unsigned>(choice.mExpectedValue)}).mValue;
    }
    else if (choice.mState == BAK::ActiveStateFlag::NightTime)
    {
        const auto time = GetWorldTime().GetTime();
        const auto hour = time.GetHour();
        const bool result = hour < 4 || hour >= 20;
        mLogger.Debug() << "Checking NightTime choice: " << time << " hr: " << hour << " -- " << result << "\n";
        return result;
    }
    else if (choice.mState == BAK::ActiveStateFlag::DayTime)
    {
        const auto time = GetWorldTime().GetTime();
        const auto hour = time.GetHour();
        const bool result = hour >= 4 && hour < 20;
        mLogger.Debug() << "Checking DayTime choice: " << time << " hr: " << hour << " -- " << result << "\n";
        return result;
    }
    else if (choice.mState == BAK::ActiveStateFlag::TimeBetween)
    {
        const auto time = GetWorldTime().GetTime();
        const auto hour = time.GetHour();
        return hour >= choice.mExpectedValue && hour <= choice.mExpectedValue2;
    }
    else if (choice.mState == BAK::ActiveStateFlag::Shop)
    {
        return GetShopType() == choice.mExpectedValue;
    }
    else if (static_cast<unsigned>(choice.mState) == 0x753f)
    {
        // This is set by the cheat screen dialog which raises
        // skill values
        //return (mContextVar_753f == choice.mExpectedValue);
        return true;
    }

    return false;
}

bool GameState::EvaluateComplexChoice(const ComplexEventChoice& choice) const
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

bool GameState::EvaluateDialogChoice(const Choice& choice) const
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
            return GetSpellActive(static_cast<StaticSpells>(c.mRequiredSpell));
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

unsigned GameState::GetEventState(unsigned eventPtr) const
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

bool GameState::GetEventStateBool(unsigned eventPtr) const
{
    return (GetEventState(eventPtr) & 0x1) == 1;
}

void GameState::SetEventValue(unsigned eventPtr, unsigned value)
{
    if (mGameData)
        State::SetEventFlag(mGameData->GetFileBuffer(), eventPtr, value);
}

void GameState::SetEventState(const SetFlag& setFlag)
{
    if (mGameData)
        State::SetEventDialogAction(mGameData->GetFileBuffer(), setFlag);
}

bool GameState::GetMoreThanOneTempleSeen() const
{
    if (mGameData)
    {
        unsigned templesSeen = 0;
        for (unsigned i = 1; i < 13; i++)
        {
            templesSeen += BAK::State::ReadTempleSeen(mGameData->GetFileBuffer(), i);
        }
        return templesSeen > 1;
    }
    return true;
}

void GameState::SetDialogContext_7530(unsigned contextValue)
{
    mContextValue_7530 = contextValue;
}

void GameState::SetItemValue(Royals value)
{
    mItemValue = value;
}

void GameState::SetInventoryItem(const InventoryItem& item)
{
    mSelectedItem = item;
}

void GameState::ClearUnseenImprovements(unsigned character)
{
    if (mGameData)
    {
        State::ClearUnseenImprovements(mGameData->GetFileBuffer(), character);
    }
}

bool GameState::SaveState()
{
    if (mGameData)
    {
        BAK::Save(GetWorldTime(), mGameData->GetFileBuffer());
        BAK::Save(GetParty(), mGameData->GetFileBuffer());

        for (const auto& container : mGDSContainers)
            BAK::Save(container, mGameData->GetFileBuffer());

        for (const auto& container : mCombatContainers)
            BAK::Save(container, mGameData->GetFileBuffer());

        for (const auto& zoneContainers : mContainers)
            for (const auto& container : zoneContainers)
                BAK::Save(container, mGameData->GetFileBuffer());
        BAK::Save(mTimeExpiringState, mGameData->GetFileBuffer());
        BAK::Save(mSpellState, mGameData->GetFileBuffer());
        BAK::Save(mChapter, mGameData->GetFileBuffer());
        return true;
    }
    return false;
}

bool GameState::Save(const SaveFile& saveFile)
{
    if (SaveState())
    {
        mGameData->Save(saveFile);
        return true;
    }
    return false;
}

bool GameState::Save(const std::string& saveName)
{
    if (SaveState())
    {
        mGameData->Save(saveName, saveName);
        return true;
    }
    return false;
}

std::vector<GenericContainer>& GameState::GetContainers(ZoneNumber zone)
{
    ASSERT(zone.mValue < 13);
    return mContainers[zone.mValue - 1];
}

bool GameState::CheckCustomStateScenarioPlagued() const
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

bool GameState::CheckCustomStateScenarioAllPartyArmorIsGoodCondition() const
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

void GameState::ElapseTime(Time time)
{
    // need to accumulate these and commit when the
    // dialog is over..?
    auto splitTime = time;
    auto camp = BAK::TimeChanger{*this};
    bool dialogResetsSleep = time > Times::TwelveHours;
    // there is further logic to this that determines
    // whether we consume rations or not.
    // e.g. cutter's gap in highcastle consumes rations,
    //      zone transitions do not...
    bool dialogConsumesRations = true;
    while (splitTime > Times::OneHour)
    {
        if (dialogResetsSleep)
        {
            GetWorldTime().SetTimeLastSlept(
                GetWorldTime().GetTime());
        }
        splitTime = splitTime - Times::OneHour;
        camp.HandleGameTimeChange(
            Times::OneHour,
            true,
            dialogConsumesRations,
            false,
            0,
            0);
    }

    if (splitTime > Time{0})
    {
        camp.HandleGameTimeChange(
            splitTime,
            true,
            dialogConsumesRations,
            false,
            0,
            0);
    }
}

void GameState::ReduceAndEvaluateTimeExpiringState(Time delta)
{
    for (unsigned i = 0; i < mTimeExpiringState.size(); i++)
    {
        auto& state = mTimeExpiringState[i];
        if (state.mDuration < delta)
        {
            state.mDuration = Time{0};
        }
        else
        {
            state.mDuration -= delta;
        }

        if (state.mType == ExpiringStateType::Light)
        {
            RecalculatePalettesForPotionOrSpellEffect(state);
        }
        else if (state.mType == ExpiringStateType::Spell)
        {
            SetSpellState(state);
        }
        else if (state.mDuration == Time{0})
        {
            if (state.mType == ExpiringStateType::SetState)
            {
                SetEventValue(state.mData, 1);
            }
            else if (state.mType == ExpiringStateType::ResetState)
            {
                SetEventValue(state.mData, 0);
            }
        }
    }

    std::erase_if(
        mTimeExpiringState,
        [](auto& state){ return state.mDuration == Time{0}; });
}

void GameState::RecalculatePalettesForPotionOrSpellEffect(TimeExpiringState& state)
{
    auto time = state.mDuration.mTime / 0x1e;
    switch (state.mData)
    {
        case 0: // torch or ring of prandur
        {
            if (time >= 8)
            {
                // paletteModifier1 = 0x31;
            }
            else
            {
                //paletteModifier1 = time * time;
            }
            if (state.mDuration == Time{0})
            {
                DeactivateLightSource();
            }
        } break;
        case 1: // dragon's breath
        {
        } break;
        case 2: // candle glow
        {
        } break;
        case 3: // stardusk
        {
        } break;
    }
}

void GameState::DeactivateLightSource()
{
    mLogger.Debug() << "Deactivating light source\n";
    GetParty().ForEachActiveCharacter([&](auto& character)
    {
        auto& items = character.GetInventory().GetItems();
        for (unsigned i = 0; i < items.size(); i++)
        {
            auto& item = items[i];
            if (item.IsActivated() 
                && (item.GetItemIndex() == sTorch 
                    || item.GetItemIndex() == sRingOfPrandur))
            {
                item.SetQuantity(item.GetQuantity() - 1);
                item.SetActivated(false);
                if (item.GetQuantity() == 0 && item.GetItemIndex() == sTorch)
                {
                    character.GetInventory().RemoveItem(InventoryIndex{i});
                }
                return true;
            }
        }
        return false;
    });
}

bool GameState::HaveActiveLightSource()
{
    for (auto& state : mTimeExpiringState)
    {
        if (state.mType == ExpiringStateType::Light
            && state.mData == 0)
        {
            return true;
        }
    }
    return false;
}

void GameState::SetSpellState(const TimeExpiringState& state)
{
    if (state.mData > 9)
    {
        assert(false);
        return;
    }
    if (state.mDuration == Time{0})
    {
        mSpellState.SetSpellState(StaticSpells{state.mData}, false);
    }
    else
    {
        mSpellState.SetSpellState(StaticSpells{state.mData}, false);
    }
}

void GameState::CastStaticSpell(StaticSpells spell, Time duration)
{
    // RunDialog DialogSources::GetSpellCastDialog(static_cast<unsigned>(spell));
    auto* state = AddSpellTimeExpiringState(mTimeExpiringState, static_cast<unsigned>(spell), duration);
    assert(state);
    SetSpellState(*state);
    switch (spell)
    {
    case StaticSpells::DragonsBreath: [[fallthrough]];
    case StaticSpells::CandleGlow: [[fallthrough]];
    case StaticSpells::Stardusk:
    {
        auto* lightState = AddLightTimeExpiringState(
            mTimeExpiringState,
            static_cast<unsigned>(spell) + 1,
            duration);
        assert(lightState);
        RecalculatePalettesForPotionOrSpellEffect(*lightState);
        // UpdatePaletteIfRequired()
    } break;
    default:
        break;
    }

    // HandlePostSpellcastingReduceHealth
}


}
