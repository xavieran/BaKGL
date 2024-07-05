#include "bak/gameState.hpp"

#include "bak/coordinates.hpp"
#include "bak/dialogChoice.hpp"
#include "bak/state/customStateChoice.hpp"

#include "bak/state/dialog.hpp"
#include "bak/state/event.hpp"
#include "bak/state/encounter.hpp"
#include "bak/state/lock.hpp"
#include "bak/state/skill.hpp"
#include "bak/state/temple.hpp"

#include "bak/save.hpp"
#include "bak/spells.hpp"
#include "bak/state/dialog.hpp"
#include "bak/dialogAction.hpp"
#include "bak/time.hpp"
#include <utility>

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
    mShopType_7542{0},
    mItemValue_753e{0},
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
    mFullMap{},
    mLogger{Logging::LogState::GetLogger("GameState")}
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
    for (unsigned i = 0; i < 13; i++)
    {
        mContainers.emplace_back(mGameData->LoadContainers(i));
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

void GameState::DoSetEndOfDialogState(std::int16_t state)
{
    mEndOfDialogState = state;
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

void GameState::SetMonster(MonsterIndex monster)
{
    mCurrentMonster = monster;
}

Royals GameState::GetMoney() const
{
    if (mGameData)
        return GetParty().GetGold();

    return Royals{1000};
}

void GameState::SetLocation(Location loc)
{
    mLogger.Debug() << "SetLocation to: " << loc << "\n";
    mZone = ZoneNumber{loc.mZone};
    if (mGameData)
    {
        mGameData->mLocation = loc;
    }
}

void GameState::SetLocation(GamePositionAndHeading posAndHeading)
{
    if (mGameData)
    {
        const auto loc = Location{
            mZone,
            GetTile(posAndHeading.mPosition),
            posAndHeading};
        mGameData->mLocation = loc;
    }
}

void GameState::SetMapLocation(MapLocation location) const
{
    mLogger.Info() << "Setting map location to: " << location << " from : " << mGameData->mMapLocation << "\n";
    if (mGameData)
        mGameData->mMapLocation = location;
}

GamePositionAndHeading GameState::GetLocation() const
{
    if (mGameData)
        return mGameData->mLocation.mLocation;

    return GamePositionAndHeading{glm::uvec2{10 * 64000, 15 * 64000}, 0 };
}

MapLocation GameState::GetMapLocation() const
{
    if (mGameData)
        return mGameData->mMapLocation;

    return MapLocation{{20, 20}, 0};
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

void GameState::SetTransitionChapter_7541(bool value)
{
    mTransitionChapter_7541 = value;
}

bool GameState::GetTransitionChapter_7541() const
{
    return mTransitionChapter_7541;
}

void GameState::SetShopType_7542(unsigned shopType)
{
    mShopType_7542 = shopType;
}

unsigned GameState::GetShopType_7542() const
{
    return mShopType_7542;
}

GenericContainer* GameState::GetContainerForGDSScene(HotspotRef ref)
{
    for (auto& shop : mGDSContainers)
    {
        if (shop.GetHeader().GetHotspotRef() == ref)
            return &shop;
    }
    return nullptr;
}

GenericContainer* GameState::GetWorldContainer(ZoneNumber zone, GamePosition location)
{
    for (auto& container : GetContainers(zone))
    {
        if (container.GetHeader().GetPosition() == location)
        {
            return &container;
        }
    }
    return nullptr;
}

GenericContainer const* GameState::GetWorldContainer(ZoneNumber zone, GamePosition location) const
{
    for (auto& container : GetContainers(zone))
    {
        if (container.GetHeader().GetPosition() == location)
        {
            return &container;
        }
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
std::pair<CharIndex, unsigned> GameState::GetPartySkill(SkillType skill, bool best)
{
    const auto [character, value] = GetParty().GetSkill(skill, best);
    mSkillCheckedCharacter = character;
    mSkillValue = value;
    return std::make_pair(character, value);
}

void GameState::SetCharacterTextVariables()
{
    DoSetEndOfDialogState(0);

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
    mLogger.Spam() << __FUNCTION__ << "(" << index << ", " << attribute << ")\n";
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
        mDialogCharacterList[index] = attribute - 1;
        mTextVariableStore.SetTextVariable(index, GetParty().GetCharacter(CharIndex{attribute - 1}).GetName());
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
            ToShopDialogString(mItemValue_753e));
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
        //    ToString(SkillType(mContextVar_753f)));
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
        [&](const SetFlag& set)
        {
            mLogger.Debug() << "Setting flag of event: " << DialogAction{set} << "\n";
            SetEventState(set);
        },
        [&](const SetTextVariable& set)
        {
            mLogger.Debug() << "Setting text variable: " << DialogAction{set} << "\n";
            SetDialogTextVariable(set.mWhich, set.mWhat);
        },
        [&](const LoseItem& lose)
        {
            mLogger.Debug() << "Losing item: " << lose << "\n";
            auto& party = GetParty();
            party.RemoveItem(lose.mItemIndex, lose.mQuantity);
        },
        [&](const GiveItem& give)
        {
            mLogger.Debug() << "Giving item: " << give << "\n";
            if (give.mWho <= 1)
            {
                auto& party = GetParty();
                party.GainItem(give.mItemIndex, give.mQuantity);
            }
            else
            {
                auto characterToGive = mDialogCharacterList[give.mWho - 2];
                GetParty().GetCharacter(CharIndex{characterToGive}).GiveItem(
                    InventoryItemFactory::MakeItem(ItemIndex{give.mItemIndex}, give.mQuantity));
            }
        },
        [&](const GainSkill& skill)
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
        [&](const HealCharacters& heal)
        {
            if (heal.mWho <= 1)
            {
                mLogger.Debug() << "Healing party by: " << heal.mHowMuch << "\n";
                GetParty().ForEachActiveCharacter([&](auto& character){
                    HealCharacter(character.mCharacterIndex, heal.mHowMuch);
                    return Loop::Continue;
                });
            }
            else
            {
                auto character = mDialogCharacterList[heal.mWho - 2];
                mLogger.Debug() << "Healing character by amount: " << heal.mHowMuch << " for "
                    << +character << "\n";
                HealCharacter(CharIndex{character}, heal.mHowMuch);
            }
        },
        [&](const SpecialAction& action)
        {
            EvaluateSpecialAction(action);
        },
        [&](const GainCondition& cond)
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
                        return Loop::Continue;
                    });
            }
        },
        [&](const SetAddResetState& state)
        {
            mLogger.Debug() << "Setting time expiring state: " << state << "\n";
            SetEventValue(state.mEventPtr, 1);
            AddTimeExpiringState(mTimeExpiringState, ExpiringStateType::ResetState, state.mEventPtr, 0x40, state.mTimeToExpire);
        },
        [&](const ElapseTime& elapse)
        {
            mLogger.Debug() << "Elapsing time: " << elapse << "\n";
            DoElapseTime(elapse.mTime);
        },
        [&](const SetTimeExpiringState& state)
        {
            mLogger.Debug() << "Setting time expiring state2: " << state << "\n";
            AddTimeExpiringState(
                mTimeExpiringState,
                state.mType,
                state.mEventPtr,
                state.mFlags,
                state.mTimeToExpire);
        },
        [&](const SetEndOfDialogState& state)
        {
            mLogger.Debug() << "Setting end of dialog state: " << state << "\n";
            DoSetEndOfDialogState(state.mState);
        },
        [&](const UpdateCharacters& update)
        {
            GetParty().SetActiveCharacters(update.mCharacters);
        },
        [&](const LoadSkillValue& load)
        {
            mLogger.Debug() << "Loading skill value: " << load << "\n";
            GetPartySkill(load.mSkill, load.mTarget == 1);
        },
        [&](const LearnSpell& learnSpell)
        {
            mLogger.Debug() << "Learning Spell: " << learnSpell << "\n";
            GetParty().GetCharacter(CharIndex{mDialogCharacterList[learnSpell.mWho]})
                .GetSpells().SetSpell(learnSpell.mWhichSpell);
        },
        [&](const LoseNOfItem& loss)
        {
            for (unsigned i = 0; i < loss.mQuantity; i++)
            {
                GetParty().RemoveItem(loss.mItemIndex, 1);
            }
        },
        [&](const auto& a){
            mLogger.Debug() << "Doing nothing for: " << a << "\n";
        }},
        action);
}

void GameState::EvaluateSpecialAction(const SpecialAction& action)
{
    mLogger.Debug() << "Evaluating special action: " << action << "\n";
    using enum SpecialActionType;
    switch (action.mType)
    {
    case ReduceGold:
        GetParty().LoseMoney(mItemValue_753e);
        break;
    case IncreaseGold:
        GetParty().GainMoney(mItemValue_753e);
        break;
    case RepairAllEquippedArmor:
        mLogger.Debug() << "Reparing party armor\n";
        GetParty().ForEachActiveCharacter([&](auto& character)
        {
            auto armor = character.GetInventory().FindEquipped(ItemType::Armor);
            if (armor != character.GetInventory().GetItems().end())
            {
                armor->SetCondition(100);
                armor->SetRepairable(false);
            }
            return Loop::Continue;
        });
        break;
    // I don't think this is used anywhere
    case CopyStandardInnToShop0: [[fallthrough]];
    case CopyStandardInnToShop1:
    {
        auto* refInn1 = GetWorldContainer(ZoneNumber{0}, {20, 0});
        assert(refInn1);
        auto* inn1 = GetContainerForGDSScene(HotspotRef{2, 'C'});
        assert(inn1);
        inn1->GetInventory().GetItems().clear();
        inn1->GetInventory().CopyFrom(refInn1->GetInventory());
    } break;
    case Increase753f:
        mLogger.Debug() << "Increasing dialog var 753f from: " << mContextVar_753f
            << " to: " << (mContextVar_753f + action.mVar1) << "\n";
        mContextVar_753f += action.mVar1;
        break;
    case Gamble:
        DoGamble(action.mVar1, action.mVar2, action.mVar3);
        break;
    case ReturnAlcoholToShops:
    {
        auto* refInn1 = GetWorldContainer(ZoneNumber{0}, {20, 0});
        assert(refInn1);
        auto* inn1 = GetContainerForGDSScene(HotspotRef{60, 'C'});
        assert(inn1);
        inn1->GetInventory().GetItems().clear();
        inn1->GetInventory().CopyFrom(refInn1->GetInventory());

        auto* refInn2 = GetWorldContainer(ZoneNumber{0}, {30, 0});
        assert(refInn2);
        auto* inn2 = GetContainerForGDSScene(HotspotRef{64, 'C'});
        assert(inn2);
        inn2->GetInventory().GetItems().clear();
        inn2->GetInventory().CopyFrom(refInn2->GetInventory());
    } break;
    case ResetGambleValueTo:
        mBardReward_754d = action.mVar1;
        break;
    case RepairAndBlessEquippedSwords:
        GetParty().ForEachActiveCharacter([&](auto& character)
        {
            auto sword = character.GetInventory().FindEquipped(ItemType::Sword);
            if (sword != character.GetInventory().GetItems().end())
            {
                sword->SetCondition(100);
                sword->SetRepairable(false);
                sword->SetModifier(Modifier::Blessing3);
            }
            return Loop::Continue;
        });
        break;
    case ExtinguishAllLightSources:
    {
        for (auto& state : mTimeExpiringState)
        {
            if (state.mType == ExpiringStateType::Light
                && state.mData == 0)
            {
                state.mDuration = Time{0};
            }
        }
        ReduceAndEvaluateTimeExpiringState(Time{0});
    } break;
    case EmptyArlieContainer:
    {
        auto* container = GetWorldContainer(ZoneNumber{3}, GamePosition{1308000, 1002400});
        assert(container);
        container->GetInventory().GetItems().clear();
    } break;
    case UnifyOwynAndPugsSpells:
    {
        auto& pug = GetParty().GetCharacter(Pug);
        auto& owyn = GetParty().GetCharacter(Owyn);
        const auto pugSpells = pug.GetSpells().GetSpellBytes();
        const auto owynSpells = owyn.GetSpells().GetSpellBytes();
        const auto allSpells = pugSpells | owynSpells;
        for (unsigned i = 0; i < 0x2d; i++)
        {
			if (CheckBitSet(allSpells, i))
            {
                pug.GetSpells().SetSpell(SpellIndex{i});
                owyn.GetSpells().SetSpell(SpellIndex{i});
            }
        }
    } break;
    default:
        mLogger.Debug() << "Unhandled action:" << action << "\n";
    }
}

void GameState::DoGamble(unsigned playerChance, unsigned gamblerChance, unsigned reward)
{
    mContextVar_753f = GetRandomNumber(0, 0xfff) % playerChance;
    mShopType_7542 = GetRandomNumber(0, 0xfff) % gamblerChance;
    mLogger.Debug() << "Rolled for player: " << mContextVar_753f
        << " for gambler: " << mShopType_7542 << "\n";

    if (mContextVar_753f <= mShopType_7542)
    {
        mLogger.Debug() << "Gambler won\n";
        if (mContextVar_753f >= mShopType_7542)
        {
            mLogger.Debug() << "Drew with gambler\n";
            if (mContextVar_753f == mShopType_7542)
            {
                SetDialogContext_7530(2);
            }
        }
        else
        {
            mLogger.Debug() << "We lost " << mItemValue_753e << "\n";
            SetDialogContext_7530(1);
            GetParty().LoseMoney(mItemValue_753e);
            if (mBardReward_754d <= 60000)
            {
                mBardReward_754d += mItemValue_753e.mValue;
            }
        }
    }
    else
    {
        mLogger.Debug() << "We won " << mItemValue_753e << "\n";
        SetDialogContext_7530(0);
        const auto winnings = Royals{(mItemValue_753e.mValue * reward) / 100};
        GetParty().GainMoney(winnings);
        mBardReward_754d = (winnings.mValue > mBardReward_754d)
            ? 0
            : (mBardReward_754d - winnings.mValue);
        //auto& character = GetParty().GetCharacter(mSelectedCharacter);
    }
}

unsigned GameState::GetGameState(const GameStateChoice& choice) const
{
    switch (choice.mState)
    {
    case ActiveStateFlag::Chapter: return GetChapter().mValue;
    case ActiveStateFlag::Context: return mContextValue_7530;
    case ActiveStateFlag::CantAfford: return static_cast<unsigned>(GetMoney() > mItemValue_753e);
    case ActiveStateFlag::SkillCheck: return mSkillValue;
    case ActiveStateFlag::Money: return Sovereigns{GetMoney().mValue}.mValue;;
    case ActiveStateFlag::TimeBetween: return GetWorldTime().GetTime().GetHour();
    case ActiveStateFlag::NightTime:
    {
        const auto time = GetWorldTime().GetTime();
        const auto hour = time.GetHour();
        const unsigned result = hour < 4 || hour >= 20;
        mLogger.Debug() << "Checking NightTime choice: " << time << " hr: " << hour << " -- " << result << "\n";
        return result;
    }
    case ActiveStateFlag::DayTime:
    {
        const auto time = GetWorldTime().GetTime();
        const auto hour = time.GetHour();
        const unsigned result = hour >= 4 && hour < 20;
        mLogger.Debug() << "Checking DayTime choice: " << time << " hr: " << hour << " -- " << result << "\n";
        return result;
    }
    case ActiveStateFlag::Shop: return GetShopType_7542();
    case ActiveStateFlag::Context_753f: return mContextVar_753f;
    case ActiveStateFlag::Gambler: return static_cast<unsigned>(mBardReward_754d);
    case ActiveStateFlag::ItemValue_753e: return mItemValue_753e.mValue;
    default: return 0u;
    }
}

bool GameState::EvaluateDialogChoice(const DialogChoice& choice) const
{
    if (std::holds_alternative<NoChoice>(choice.mChoice))
    {
        return true;
    }
    else if (!std::holds_alternative<ComplexEventChoice>(choice.mChoice))
    {
        const unsigned state = GetEventState(choice.mChoice);
        auto result = state >= choice.mMin 
            && ((choice.mMax == 0xffff) 
                || (state <= choice.mMax));
        mLogger.Info() << "Evaluating choice: "<< choice << " state is: " << state
            << "result: " << result << "\n";
        return result;
    }
    else
    {
        const auto& complexChoice = std::get<ComplexEventChoice>(choice.mChoice);
        const unsigned state = ReadEvent(complexChoice.mEventPointer);
        // Probably want to put this logic somewhere else...
        // if eventPtr % 10 != 0
        const auto [byteOffset, bitOffset] = State::CalculateComplexEventOffset(complexChoice.mEventPointer);
        mLogger.Debug() << __FUNCTION__ << " : " << choice 
            << " S: [" << std::hex << +state << std::dec << "] - byteOff: " 
            << + byteOffset << " bitOff: " << +bitOffset << "\n";

        const auto xorMask = static_cast<std::uint8_t>(choice.mMin & 0xff);
        const auto expected = static_cast<std::uint8_t>(choice.mMin >> 8);
        const auto mustEqualExpected = static_cast<std::uint8_t>(choice.mMax& 0xff);
        const auto chapterMask = static_cast<std::uint8_t>(choice.mMax >> 8);
        //static_cast<std::uint8_t>(choice1 & 0xff),
        //static_cast<std::uint8_t>(choice1 >> 8)

        if (mGameData && bitOffset != 0)
        {
            return (state >= xorMask) && (state <= mustEqualExpected);
        }

        // Need to double check this...
        const auto chapterFlag = GetChapter() == Chapter{9}
            ? 0x80
            : 1 << (GetChapter().mValue - 1);
        //const auto chapterMaskSatisfied = (chapterFlag & ChapterMask) == 0;
        const auto chapterMaskSatisfied = true;

        if (mustEqualExpected)
        {
            if (((state ^ xorMask) & expected) == expected
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
            if (((state ^ xorMask) & expected) != 0
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
}

unsigned GameState::GetEventState(Choice choice) const
{
    return std::visit(overloaded{
        [&](const NoChoice& c) -> unsigned {
            return true;
        },
        [&](const EventFlagChoice& c) -> unsigned {
            return ReadEventBool(c.mEventPointer);
        },
        [&](const ComplexEventChoice& c) -> unsigned {
            return ReadEventBool(c.mEventPointer);
        },
        [&](const InventoryChoice& c) -> unsigned {
            return GetParty().HaveItem(c.mRequiredItem);
        },
        [&](const GameStateChoice& c) -> unsigned {
            return GetGameState(c);
        },
        [&](const CastSpellChoice& c) -> unsigned {
            return GetSpellActive(static_cast<StaticSpells>(c.mRequiredSpell));
        },
        [&](const HaveNoteChoice& c) -> unsigned {
            return HaveNote(c.mRequiredNote);
        },
        [&](const CustomStateChoice& c) -> unsigned {
            const auto stateChecker = State::CustomStateEvaluator{*this};
            switch (c.mScenario)
            {
            case Scenario::MortificationOfTheFlesh:
                return stateChecker.AnyCharacterStarving();
            case Scenario::Plagued:
                return stateChecker.Plagued();
            case Scenario::HaveSixSuitsOfArmor:
                return stateChecker.HaveSixSuitsOfArmor();
            case Scenario::AllPartyArmorIsGoodCondition:
                return stateChecker.AllPartyArmorIsGoodCondition();
            case Scenario::PoisonedDelekhanArmyChests:
                return stateChecker.PoisonedDelekhanArmyChests();
            case Scenario::AnyCharacterSansWeapon:
                return stateChecker.AnyCharacterSansWeapon();
            case Scenario::AlwaysFalse: [[fallthrough]];
            case Scenario::AlwaysFalse2:
                return false;
            case Scenario::AnyCharacterHasNegativeCondition:
                return stateChecker.AnyCharacterHasNegativeCondition();
            case Scenario::AllPartyMembersHaveNapthaMask:
                return stateChecker.AllCharactersHaveNapthaMask();
            case Scenario::NormalFoodInArlieChest:
                return stateChecker.NormalFoodInArlieChest();
            case Scenario::PoisonedFoodInArlieChest:
                return stateChecker.PoisonedFoodInArlieChest();
            default:
                return false;
            }
        },
        [&](const RandomChoice& c){
            return GetRandomNumber(0, 0xfff) % c.mRange;
        },
        [&](const auto& c) -> unsigned {
            return false; 
        }
    },
    choice);
}

unsigned GameState::ReadEvent(unsigned eventPtr) const
{
    if (eventPtr == std::to_underlying(ActiveStateFlag::DayTime))
    {
        return GetGameState(GameStateChoice{ActiveStateFlag::DayTime});
    }
    if (mGameData != nullptr)
    {
        return State::ReadEvent(mGameData->GetFileBuffer(), eventPtr);
    }
    else
        return 0;
}

bool GameState::ReadEventBool(unsigned eventPtr) const
{
    return (ReadEvent(eventPtr) & 0x1) == 1;
}

void GameState::SetEventValue(unsigned eventPtr, unsigned value)
{
    if (eventPtr == 0x7541)
    {
        mTransitionChapter_7541 = value;
        return;
    }

    if (mGameData)
        State::SetEventFlag(mGameData->GetFileBuffer(), eventPtr, value);
}

void GameState::SetEventState(const SetFlag& setFlag)
{
    if (setFlag.mEventPointer == 0x753e)
    {
        mItemValue_753e = Royals{setFlag.mEventValue};
    }
    else if (setFlag.mEventPointer == 0x753f)
    {
        mContextVar_753f = setFlag.mEventValue;
    }
    else if (setFlag.mEventPointer == 0x7541)
    {
        mTransitionChapter_7541 = setFlag.mEventValue;
    }
    else if (mGameData)
    {
        State::SetEventDialogAction(mGameData->GetFileBuffer(), setFlag);
    }
}

bool GameState::GetMoreThanOneTempleSeen() const
{
    if (mGameData)
    {
        unsigned templesSeen = 0;
        for (unsigned i = 1; i < 13; i++)
        {
            templesSeen += State::ReadTempleSeen(*this, i);
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
    mItemValue_753e = value;
}

void GameState::SetBardReward_754d(unsigned value)
{
    mBardReward_754d = value;
}

unsigned GameState::GetBardReward_754d()
{
    return mBardReward_754d;
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
        auto mapLocation = MapLocation{
            mFullMap.GetTileCoords(mZone, GetTile(GetLocation().mPosition)),
            HeadingToFullMapAngle(GetLocation().mHeading)
        };
        BAK::Save(mapLocation, mGameData->GetFileBuffer());
        BAK::Save(mGameData->mLocation, mGameData->GetFileBuffer());
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
    return mContainers[zone.mValue];
}

const std::vector<GenericContainer>& GameState::GetContainers(ZoneNumber zone) const
{
    ASSERT(zone.mValue < 13);
    return mContainers[zone.mValue];
}


bool GameState::HaveNote(unsigned note) const
{
    bool haveNote = false;
    GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            const auto& items = character.GetInventory().GetItems();
            for (const auto& item : items)
            {
                if (item.IsItemType(ItemType::Note) && item.GetCondition() == note)
                {
                    haveNote = true;
                    return Loop::Finish;
                }
            }
            return Loop::Continue;
        });
    return haveNote;
}

bool GameState::CheckConversationItemAvailable(unsigned conversationItem) const
{
    const auto stateChecker = State::CustomStateEvaluator{*this};
    const bool enabled = ReadEventBool(conversationItem);
    const bool available = std::invoke([&](){
        switch (conversationItem)
        {
        case 9:
            if (!enabled) return false;
            return !GetParty().HaveItem(sWaani);
        case 11:
            if (!enabled) return false;
            return !GetParty().HaveItem(sBagOfGrain);
        case 133:
            return GetEventState(CreateChoice(0xdb94)) != 0;
        case 130:
            return GetEventState(CreateChoice(0xdb9e)) != 0;
        case 44:
            if (!enabled) return false;
            return ReadEventBool(0x1f6c);
        case 117:
            if (!enabled) return false;
            return GetChapter() == Chapter{6};
        case 17: [[fallthrough]];
        case 103:
            if (!enabled) return false;
            return stateChecker.AnyCharacterHasNegativeCondition();
        case 71:
            if (!enabled) return false;
            return false; // (owynsSpells & 0x10) != 0; (Have spell 20 (Unfortunate flux))
        case 132:
            if (!enabled) return false;
            return HaveNote(0x15) || ReadEventBool(0x1979);
        case 106:
            if (!enabled) return false;
            return true; // owynsOtherSpells & 0x200
        case 164:
            if (!enabled) return false;
            return ReadEventBool(0x1972);
        case 76: [[fallthrough]];
        case 148:
            if (!enabled) return false;
            return !GetParty().HaveItem(sRations);
        case 163:
            return ReadEventBool(0x8e)
                && ReadEventBool(0xaa)
                && GetParty().HaveItem(sAbbotsJournal);
        default:
            return enabled;
        }
    });
    return available && !State::CheckConversationOptionInhibited(*this, conversationItem);
}

void GameState::DoElapseTime(Time time)
{
    // need to accumulate these and commit when the
    // dialog is over..?
    auto splitTime = time;
    auto camp = TimeChanger{*this};
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
                return Loop::Finish;
            }
        }
        return Loop::Continue;
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
        mSpellState.SetSpellState(StaticSpells{state.mData}, true);
    }
}

void GameState::CastStaticSpell(StaticSpells spell, Time duration)
{
    mLogger.Debug() << "Casting spell: " << spell << " for: " << duration << "\n";
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
    mLogger.Debug() << "State now: " << mTimeExpiringState << "\n";
    mLogger.Debug() << "SpellState now: " << std::hex << mSpellState.GetSpells() << std::dec << "\n";

    // HandlePostSpellcastingReduceHealth
}


bool GameState::CanCastSpell(SpellIndex spell, ActiveCharIndex activeChar)
{
    const auto& character = GetParty().GetCharacter(activeChar);
    // FIXME: Add additional conditions, e.g. Stardusk only works outdoors...
    const auto& spellInfo = SpellDatabase::Get().GetSpell(spell);
    if (character.GetSkill(SkillType::TotalHealth) < spellInfo.mMinCost)
        return false;

    return character.GetSpells().HaveSpell(spell);
}

void GameState::HealCharacter(CharIndex who, unsigned amount)
{
    auto& character = GetParty().GetCharacter(who);
    if (amount >= 0x64)
    {
        GetWorldTime().SetTimeLastSlept(GetWorldTime().GetTime());
        for (unsigned i = 0; i < 7; i++)
        {
            character.AdjustCondition(static_cast<Condition>(i), -100);
        }
        character.ImproveSkill(
            SkillType::TotalHealth,
            SkillChange::HealMultiplier_100,
            0x7fff);
    }
    else
    { // Start of Chapter 4, reduces health by 20%
        const int currentHealth = character.GetSkill(SkillType::TotalHealth);
        const auto reduction = ((currentHealth * -20) / 100) << 8;
        character.ImproveSkill(
            SkillType::TotalHealth,
            SkillChange::HealMultiplier_100,
            reduction);
    }
}
}
