#include "game/interactable/combatant.hpp"

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

Combatant::Combatant(
    Gui::IGuiManager& guiManager,
    BAK::Target target)
:
    mGuiManager{guiManager},
    mDialogScene{
        []{},
        []{},
        [&](const auto& choice){ DialogFinished(choice); }},
    mDefaultDialog{target},
    mContainer{nullptr}
{}

void Combatant::BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType)
{
    mContainer = &container;
    mEntityType = entityType;

    StartDialog(BAK::DialogSources::mBody);
}

void Combatant::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mContainer);
    if (mContainer->HasInventory())
    {
        mGuiManager.ShowContainer(mContainer, BAK::EntityType::DEADBODY1);
    }
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
