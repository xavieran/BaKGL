#include "game/interactable/description.hpp"

#include "game/interactable/IInteractable.hpp"

#include "bak/container.hpp"

#include "gui/IGuiManager.hpp"

namespace Game::Interactable {

Description::Description(
    Gui::IGuiManager& guiManager,
    BAK::Target target)
:
    mGuiManager{guiManager},
    mDialogScene{
        []{},
        []{},
        [](const auto&){}},
    mDialog{target}
{}

void Description::BeginInteraction(BAK::GenericContainer&, BAK::EntityType)
{
    mGuiManager.StartDialog(mDialog, false, false, &mDialogScene);
}

void Description::EncounterFinished() {}

}
