#include "game/interactable/corpse.hpp"

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

Corpse::Corpse(
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

void Corpse::BeginInteraction(BAK::GenericContainer& container, BAK::EntityType entityType)
{
    mContainer = &container;
    mEntityType = entityType;

    StartDialog(BAK::DialogSources::mBody);
}

void Corpse::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mContainer);
    if (mContainer->HasInventory())
    {
        mGuiManager.ShowContainer(mContainer, BAK::EntityType::DEADBODY1);
    }
}

void Corpse::EncounterFinished()
{
}

void Corpse::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

}
