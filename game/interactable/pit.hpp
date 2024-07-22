#pragma once

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

class Pit : public IInteractable
{
public:
    Pit(
        Gui::IGuiManager& guiManager,
        BAK::GameState& gameState)
    :
        mGuiManager{guiManager},
        mGameState{gameState},
        mDialogScene{
            []{},
            []{},
            [&](const auto& choice){ DialogFinished(choice); }},
        mCurrentPit{nullptr}
    {}

    void BeginInteraction(BAK::GenericContainer& pit, BAK::EntityType) override
    {
        mCurrentPit = &pit;
        if (mGameState.GetParty().HaveItem(BAK::sRope))
        {
            StartDialog(BAK::DialogSources::mPitHaveRope);
        }
        else
        {
            StartDialog(BAK::DialogSources::mPitNoRope);
        }
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
    {
        ASSERT(mCurrentPit);
        if (choice && choice->mValue == BAK::Keywords::sYesIndex)
        {
            Logging::LogDebug(__FUNCTION__) << "Swing across pit...\n";
        }
        else
        {
            Logging::LogDebug(__FUNCTION__) << "Not crossing pit\n";
        }
    }

    void StartDialog(BAK::Target target)
    {
        mGuiManager.StartDialog(
            target,
            false,
            false,
            &mDialogScene);
    }

    void EncounterFinished() override
    {
    }

private:
    Gui::IGuiManager& mGuiManager;
    BAK::GameState& mGameState;
    Gui::DynamicDialogScene mDialogScene;
    BAK::GenericContainer* mCurrentPit;
};

}
