#include "game/interactable/generic.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/IContainer.hpp"
#include "bak/container.hpp"
#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/types.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

Generic::Generic(
    Gui::IGuiManager& guiManager,
    BAK::Target target,
    const EncounterCallback& encounterCallback)
:
    mGuiManager{guiManager},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mDefaultDialog{target},
    mContainer{nullptr},
    mEncounterCallback{encounterCallback}
{}

void Generic::BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType)
{
    mContainer = &container;
    mEntityType = entityType;

    if (container.HasDialog())
        StartDialog(container.GetDialog().mDialog);
    else if (container.HasEncounter())
        DoEncounter();
    else
        StartDialog(mDefaultDialog);
}

void Generic::DoEncounter()
{
    std::invoke(
        mEncounterCallback,
        *mContainer->GetEncounter().mEncounterPos);
}

void Generic::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mContainer);
    if (mContainer->HasInventory())
    {
        mGuiManager.ShowContainer(mContainer, mEntityType);
    }
}

void Generic::EncounterFinished()
{
}

void Generic::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

}
