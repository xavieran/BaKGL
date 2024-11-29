#include "game/interactable/combatant.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/constants.hpp"
#include "bak/encounter/combat.cpp"
#include "bak/container.hpp"
#include "bak/dialogSources.hpp"
#include "bak/state/encounter.hpp"
#include "bak/gameState.hpp"
#include "bak/types.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

Combatant::Combatant(
    Gui::IGuiManager& guiManager,
    BAK::GameState& gameState)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mContainer{nullptr}
{}

void Combatant::BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType)
{
    mContainer = &container;
    mEntityType = entityType;

    const auto combatNumber = container.GetHeader().GetCombatNumber();
    
    if (combatNumber == 64)
    {
        auto factory = BAK::Encounter::GenericCombatFactory<false>{};
        // fix this so the combatant has this info already.
        const auto& combat = factory.Get(220);
        StartDialog(combat.mEntryDialog);
        return;
    }
    else if (combatNumber == 63)
    {
        StartDialog(BAK::DialogSources::mWeCantTakeAllTheseFellas);
        return;
    }

    auto combatPlannedTime = mGameState.Apply(BAK::State::GetCombatClickedTime, combatNumber);
    const bool alreadyPlannedAttack = (mGameState.GetWorldTime().GetTime() - combatPlannedTime) < BAK::Times::OneDay;
    if (!alreadyPlannedAttack)
    {
        mGameState.Apply(BAK::State::SetCombatClickedTime, combatNumber, mGameState.GetWorldTime().GetTime());
    }

    StartDialog(alreadyPlannedAttack
        ? BAK::DialogSources::mAlreadyPlannedAttack
        : BAK::DialogSources::mPlanAttack);
}

void Combatant::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mContainer);
}

void Combatant::EncounterFinished()
{
}

void Combatant::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

}
