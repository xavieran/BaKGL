#include "game/encounterHandler.hpp"

#include "bak/camera.hpp"
#include "bak/dialog.hpp"
#include "bak/encounter/block.hpp"
#include "bak/encounter/combat.hpp"
#include "bak/encounter/dialog.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/encounter/eventFlag.hpp"
#include "bak/encounter/gdsEntry.hpp"
#include "bak/encounter/zone.hpp"
#include "bak/gameState.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/state/encounter.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "gui/IGuiManager.hpp"

#include <cassert>
#include <utility>

namespace Game {

EncounterHandler::EncounterHandler(
    BAK::GameState& gameState,
    Gui::IGuiManager& guiManager,
    Camera& camera)
:
    mGameState{gameState},
    mGuiManager{guiManager},
    mCamera{camera},
    mDynamicDialogScene{
        [&](){ mCamera.SetAngle(mSavedAngle); },
        [&](){ mCamera.SetAngle(mSavedAngle + glm::vec2{3.14, 0}); },
        [&](const auto&){ }
    },
    mCombatHandler{mGameState, mGuiManager, mDynamicDialogScene},
    mSavedAngle{0},
    mTransitionCallback{},
    mLogger{Logging::LogState::GetLogger("Game::EncounterHandler")}
{}

void EncounterHandler::SetTransitionCallback(TransitionCallback&& callback)
{
    mTransitionCallback = std::move(callback);
}

void EncounterHandler::DoEncounter(const BAK::Encounter::Encounter& encounter)
{
    mLogger.Spam() << "Doing Encounter: " << encounter << "\n";
    std::visit(
        overloaded{
        [&](const BAK::Encounter::GDSEntry& gds){
            if (mGuiManager.InMainView())
                DoGDSEncounter(encounter, gds);
        },
        [&](const BAK::Encounter::Block& block){
            if (mGuiManager.InMainView())
                DoBlockEncounter(encounter, block);
        },
        [&](const BAK::Encounter::Combat& combat){
            if (mGuiManager.InMainView())
                mCombatHandler.CheckAndDoCombatEncounter(encounter, combat);
        },
        [&](const BAK::Encounter::Dialog& dialog){
            if (mGuiManager.InMainView())
                DoDialogEncounter(encounter, dialog);
        },
        [&](const BAK::Encounter::EventFlag& flag){
            if (mGuiManager.InMainView())
                DoEventFlagEncounter(encounter, flag);
        },
        [&](const BAK::Encounter::Zone& zone){
            if (mGuiManager.InMainView())
                DoZoneEncounter(encounter, zone);
        },
    },
    encounter.GetEncounter());
}

CombatEncounterHandler& EncounterHandler::GetCombatHandler()
{
    return mCombatHandler;
}

void EncounterHandler::DoBlockEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Block& block)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;

    mGuiManager.StartDialog(
            block.mDialog,
            false,
            false,
            &mDynamicDialogScene);

    mCamera.UndoPositionChange();
    mGameState.Apply(
        BAK::State::SetPostEnableOrDisableEventFlags,
        encounter,
        mGameState.GetZone());
}

void EncounterHandler::DoEventFlagEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::EventFlag& flag)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;

    if (flag.mEventPointer != 0)
        mGameState.SetEventValue(flag.mEventPointer, flag.mIsEnable ? 1 : 0);

    mGameState.Apply(
        BAK::State::SetPostEnableOrDisableEventFlags,
        encounter,
        mGameState.GetZone());
}

void EncounterHandler::DoZoneEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Zone& zone)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;
    const auto& choices = BAK::DialogStore::Get().GetSnippet(zone.mDialog).mChoices;
    const bool isNoAffirmative = choices.size() == 2
        && std::holds_alternative<BAK::QueryChoice>(choices.begin()->mChoice)
        && std::get<BAK::QueryChoice>(choices.begin()->mChoice).mQueryIndex == BAK::Keywords::sNoIndex;
    mDynamicDialogScene.SetDialogFinished(
        [&, isNoAffirmative=isNoAffirmative, zone=zone](const auto& choice){
            // These dialogs should always result in a choice
            ASSERT(choice);
            Logging::LogDebug("Game::EncounterHandler") << "Switch to zone: " << zone << " got choice: " << choice << "\n";
            if ((choice->mValue == BAK::Keywords::sYesIndex && !isNoAffirmative)
                || (choice->mValue == BAK::Keywords::sNoIndex && isNoAffirmative))
            {
                ASSERT(mTransitionCallback);
                mTransitionCallback(
                    zone.mTargetZone,
                    zone.mTargetLocation);
                Logging::LogDebug("Game::EncounterHandler") << "Transition to: " << zone.mTargetZone << " complete\n";
            }
            else
            {
                mCamera.UndoPositionChange();
            }
            mDynamicDialogScene.ResetDialogFinished();
        });
    mLogger.Info() << "Zone transition: " << zone << "\n";
    mGuiManager.StartDialog(
        zone.mDialog,
        false,
        false,
        &mDynamicDialogScene);
}

void EncounterHandler::DoDialogEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Dialog& dialog)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;

    mSavedAngle = mCamera.GetAngle();
    mDynamicDialogScene.SetDialogFinished(
        [&](const auto&){
            mCamera.SetAngle(mSavedAngle);
            mDynamicDialogScene.ResetDialogFinished();
        });

    mGuiManager.StartDialog(
        dialog.mDialog,
        false,
        true,
        &mDynamicDialogScene);

    mGameState.Apply(
        BAK::State::SetPostDialogEventFlags,
        encounter,
        mGameState.GetZone());
}

void EncounterHandler::DoGDSEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::GDSEntry& gds)
{
    // Pretty sure GDS encoutners will always happen...
    //if (!mGameState.Apply(BAK::State::CheckEncounterActive, encounter, mGameState.GetZone()))
    //    return;

    mDynamicDialogScene.SetDialogFinished(
        [&, gds=gds](const auto& choice){
            ASSERT(choice);
            if (choice->mValue == BAK::Keywords::sYesIndex)
            {
                mGuiManager.DoFade(.8, [this, gds=gds]{
                    mGuiManager.EnterGDSScene(
                        gds.mHotspot,
                        [&, exitDialog=gds.mExitDialog](){
                            mGuiManager.StartDialog(
                                exitDialog,
                                false,
                                false,
                                &mDynamicDialogScene);
                            });
                            mCamera.SetGameLocation(gds.mExitPosition);
                });
            }
            else
            {
                mCamera.UndoPositionChange();
            }

            mGameState.Apply(BAK::State::SetPostGDSEventFlags, encounter);
            mDynamicDialogScene.ResetDialogFinished();
        });

    mGuiManager.StartDialog(
        gds.mEntryDialog,
        false,
        false,
        &mDynamicDialogScene);
}

}
