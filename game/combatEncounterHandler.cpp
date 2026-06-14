#include "game/combatEncounterHandler.hpp"

#include "bak/encounter/combat.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/party.hpp"
#include "bak/spells.hpp"
#include "bak/state/encounter.hpp"
#include "bak/state/event.hpp"
#include "bak/time.hpp"

#include "com/logger.hpp"

#include "gui/IGuiManager.hpp"

namespace Game {

CombatEncounterHandler::CombatEncounterHandler(
    BAK::GameState& gameState,
    Gui::IGuiManager& guiManager,
    Gui::DynamicDialogScene& dynamicDialogScene)
:
    mGameState{gameState},
    mGuiManager{guiManager},
    mDynamicDialogScene{dynamicDialogScene},
    mEnterCombatCallback{},
    mLogger{Logging::LogState::GetLogger("Game::CombatEncounterHandler")}
{}

void CombatEncounterHandler::SetEnterCombatCallback(std::function<void()>&& callback)
{
    mEnterCombatCallback = std::move(callback);
}

bool CombatEncounterHandler::CombatIsUnavoidable(BAK::CombatIndex combatIndex)
{
    if (mGameState.GetCombatTriggeredFromInteractable())
    {
        return true;
    }

    switch (combatIndex.mValue)
    {
        case 151: [[fallthrough]];
        case 152: [[fallthrough]];
        case 235: [[fallthrough]];
        case 245: [[fallthrough]];
        case 291: [[fallthrough]];
        case 293: [[fallthrough]];
        case 335: [[fallthrough]];
        case 337: [[fallthrough]];
        case 338: [[fallthrough]];
        case 375: [[fallthrough]];
        case 410: [[fallthrough]];
        case 429: [[fallthrough]];
        case 430:
            return true;
    }
    return false;
}

CombatCheckResult CombatEncounterHandler::CheckCombatEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Combat& combat)
{
    mLogger.Debug() << __FUNCTION__ << " Checking combat active\n";
    if (!BAK::State::CheckCombatActive(mGameState, encounter, mGameState.GetZone()))
    {
        mLogger.Debug() << __FUNCTION__ << " Combat inactive\n";
        return CombatCheckResult(false, false);
    }

    if (CombatIsUnavoidable(combat.mCombatIndex))
    {
        mLogger.Debug() << __FUNCTION__ << " Combat is unavoidable\n";
        return CombatCheckResult(true, false);
    }


    if (!combat.mIsAmbush)
    {
        mLogger.Debug() << __FUNCTION__ << " Combat is not ambush\n";
        return CombatCheckResult(true, false);
    }
    else
    {
        // This seems to be a variable used to prevent the scouting of multiple combats
        // at once...
        const auto arg_dontDoCombatIfIsAmbush = false;
        if (arg_dontDoCombatIfIsAmbush)
        {
            return CombatCheckResult(false, false);
        }
        else
        {
            if (!BAK::State::CheckRecentlyEncountered(mGameState, encounter.GetIndex().mValue))
            {
                mGameState.Apply(BAK::State::SetRecentlyEncountered, encounter.GetIndex().mValue);
                auto chance = GetRandomNumber(0, 0xfff) % 100;
                const auto [character, scoutSkill] = mGameState.GetPartySkill(BAK::SkillType::Scouting, true);
                mLogger.Debug() << __FUNCTION__ << " Trying to scout combat: "
                    << scoutSkill << " chance: " << chance << "\n";
                if (scoutSkill >= chance)
                {
                    mGameState.GetParty().ImproveSkillForAll(
                        BAK::SkillType::Scouting, BAK::SkillChange::ExercisedSkill, 1);
                    mGuiManager.StartDialog(
                        combat.mScoutDialog,
                        false,
                        false,
                        &mDynamicDialogScene);

                    mGameState.Apply(BAK::State::SetCombatEncounterScoutedState,
                        encounter.GetIndex().mValue, true);

                    return CombatCheckResult(false, true);
                }
                else
                {
                    return CombatCheckResult(true, false);
                }
            }
            else
            {
                mLogger.Debug() << __FUNCTION__ << " Combat was scouted already\n";
                return CombatCheckResult(true, false);
            }
        }
    }
}

unsigned CombatEncounterHandler::CalculateAvoidanceStealth(
    const BAK::Encounter::Combat& combat)
{
    const auto [character, stealthSkill] = mGameState.GetPartySkill(BAK::SkillType::Stealth, false);
    auto lowestStealth = stealthSkill;
    if (lowestStealth < 0x5a) // 90
    {
        lowestStealth += (lowestStealth * 0x1e) / 100; // 30
    }
    if (lowestStealth > 0x5a) lowestStealth = 0x5a;

    if (combat.mIsAmbush)
    {
        if (mGameState.GetSpellActive(BAK::StaticSpells::DragonsBreath))
        {
            lowestStealth = lowestStealth + ((100 - lowestStealth) >> 1);
        }
    }

    return lowestStealth;
}

bool CombatEncounterHandler::CheckAvoidCombatDueToStealth(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Combat& combat)
{

    auto stealth = CalculateAvoidanceStealth(combat);
    auto chance = GetRandomNumber(0, 0xfff) % 100;
    if (stealth >= chance)
    {
        mGameState.GetParty().ImproveSkillForAll(
            BAK::SkillType::Stealth, BAK::SkillChange::ExercisedSkill, 1);

        mLogger.Debug() << __FUNCTION__ << " Avoided combat due to stealth\n";
        return true;
    }

    return false;
}

bool CombatEncounterHandler::CheckAndDoCombatEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Combat& combat)
{
    mLogger.Debug() << __FUNCTION__ << " Handling encounter: " << encounter << "\n with combat " << "\n";
    const auto [combatActive, combatScouted] = CheckCombatEncounter(encounter, combat);

    mLogger.Debug() << __FUNCTION__ << " Combat checked, result: [" << combatActive << ", " << combatScouted << "]\n";

    if (!combatActive)
    {
        mLogger.Debug() << __FUNCTION__ << " Combat not active, not doing it\n";
        return combatScouted;
    }

    if (!CombatIsUnavoidable(combat.mCombatIndex))
    {
        auto avoidedDueToStealth = CheckAvoidCombatDueToStealth(encounter, combat);
        if (avoidedDueToStealth)
        {
            return true;
        }
    }

    const auto [character, stealth] = mGameState.GetPartySkill(BAK::SkillType::Stealth, false);
    // Check whether players are in valid combatable position???
    auto timeOfScouting = mGameState.Apply(BAK::State::GetCombatClickedTime, combat.mCombatIndex);
    auto timeDiff = (mGameState.GetWorldTime().GetTime() - timeOfScouting).mTime;
    if ((timeDiff / 0x1e) < 0x1e) // within scouting valid time
    {
        auto chance = GetRandomNumber(0, 0xfff) % 100;
        if (chance > stealth)
        {
            // failed to sneak up
            // mGameState.SetActiveCharacter(character);
            mGameState.SetDialogContext_7530(1);
        }
        else
        {
            // Successfully snuck
            mGameState.GetParty().ImproveSkillForAll(
                BAK::SkillType::Stealth, BAK::SkillChange::ExercisedSkill, 1);
            mGameState.SetDialogContext_7530(0);
        }
    }
    else
    {
        mGameState.SetDialogContext_7530(2);
    }

    if (!combat.mCombatants.empty())
    {
        mGameState.SetMonster(BAK::MonsterIndex{combat.mCombatants.back().mMonster + 1u});
    }

    if (combat.mEntryDialog.mValue != 0)
    {
        // FIXME: Need to add surprise to EnterCombatScreen
        // recordTimeOfCombat at (combatIndex << 2) + 0x3967
        mGuiManager.SetCombatSequenceActive(true);
        mDynamicDialogScene.SetDialogFinished(
            [&](const auto& choice){
                Logging::LogDebug("Game::CombatEncounterHandler") << "Enter Combat\n";
                ASSERT(mEnterCombatCallback);
                mEnterCombatCallback();
            });

        mGuiManager.StartDialog(
            combat.mEntryDialog,
            false,
            false,
            &mDynamicDialogScene);
    }

    return true;
}

void CombatEncounterHandler::UpdatePostEncounterFlags(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Combat& combat)
{
    if (encounter.mCompletionState != 0)
    {
        mGameState.Apply(BAK::State::SetEventFlagTrue, encounter.mCompletionState);
    }

    mGameState.Apply(
        BAK::State::SetUniqueEncounterStateFlag,
        mGameState.GetZone(),
        encounter.GetTileIndex(),
        encounter.GetIndex().mValue,
        true);

    mGameState.Apply(
        BAK::State::SetCombatEncounterState,
        combat.mCombatIndex,
        true);

    BAK::State::SetPostCombatCombatSpecificFlags(mGameState, encounter, combat.mCombatIndex);
    // This is a part of the above function, but I separate it out here
    // to keep things cleaner. Yes this is hardcoded in the game code.
    static constexpr auto NagoCombatIndex = BAK::CombatIndex{74};
    if (combat.mCombatIndex == NagoCombatIndex)
    {
        auto afterNagoKeys = BAK::DialogStore::Get()
            .GetSnippet(BAK::DialogSources::mAfterNagoCombatSetKeys);
        for (const auto& action : afterNagoKeys.mActions)
        {
            mGameState.EvaluateAction(action);
        }
    }
}
}
