#include "game/combatEncounterHandler.hpp"

#include "bak/encounter/combat.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/gameState.hpp"
#include "bak/party.hpp"
#include "bak/spells.hpp"
#include "bak/state/encounter.hpp"
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
                if (scoutSkill > chance)
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

void CombatEncounterHandler::CheckAndDoCombatEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Combat& combat)
{
    const auto [combatActive, combatScouted] = CheckCombatEncounter(encounter, combat);

    mLogger.Debug() << __FUNCTION__ << " Combat checked, result: [" << combatActive << ", " << combatScouted << "]\n";

    if (!combatActive)
    {
        mLogger.Debug() << __FUNCTION__ << " Combat not active, not doing it\n";
        return;
    }

    const auto [character, stealthSkill] = mGameState.GetPartySkill(BAK::SkillType::Stealth, false);
    auto lowestStealth = stealthSkill;
    if (lowestStealth < 0x5a) // 90
    {
        lowestStealth *= 0x1e; // 30
        lowestStealth += (lowestStealth / 100);
    }
    if (lowestStealth > 0x5a) lowestStealth = 0x5a;

    if (combat.mIsAmbush)
    {
        if (mGameState.GetSpellActive(BAK::StaticSpells::DragonsBreath))
        {
            lowestStealth = lowestStealth + ((100 - lowestStealth) >> 1);
        }
    }

    auto chance = GetRandomNumber(0, 0xfff) % 100;
    if (lowestStealth > chance)
    {
        mGameState.GetParty().ImproveSkillForAll(
            BAK::SkillType::Stealth, BAK::SkillChange::ExercisedSkill, 1);

        mLogger.Debug() << __FUNCTION__ << " Avoided combat due to stealth\n";
        return;
    }

    // Check whether players are in valid combatable position???
    auto timeOfScouting = mGameState.Apply(BAK::State::GetCombatClickedTime, combat.mCombatIndex);
    auto timeDiff = (mGameState.GetWorldTime().GetTime() - timeOfScouting).mTime;
    if ((timeDiff / 0x1e) < 0x1e) // within scouting valid time
    {
        auto chance = GetRandomNumber(0, 0xfff) % 100;
        if (chance > lowestStealth)
        {
            // failed to sneak up
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
}

}
