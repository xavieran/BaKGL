#include "game/interactable/pit.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/entityType.hpp"
#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

Pit::Pit(
    Gui::IGuiManager& guiManager,
    BAK::GameState& gameState,
    const PitCrossCallback& pitCrossCallback)
:
    mGuiManager{guiManager},
    mGameState{gameState},
    mPitCrossCallback{pitCrossCallback},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mEntityIndex{0}
{}

void Pit::BeginInteraction(BAK::GenericContainer& pit, BAK::EntityType, BAK::EntityIndex entityIndex)
{
    mEntityIndex = entityIndex;
    if (mGameState.GetParty().HaveItem(BAK::sRope))
    {
        StartDialog(BAK::DialogSources::mPitHaveRope);
    }
    else
    {
        StartDialog(BAK::DialogSources::mPitNoRope);
    }
}

void Pit::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    if (choice && choice->mValue == BAK::Keywords::sYesIndex)
    {
        mGameState.GetParty().RemoveItem(BAK::sRope.mValue, 1);
        mPitCrossCallback(mEntityIndex);
    }
}

void Pit::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

void Pit::EncounterFinished()
{
}

}
