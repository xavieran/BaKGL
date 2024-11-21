#pragma once

#include "bak/dialogTarget.hpp"

#include "gui/IDialogScene.hpp"

namespace BAK {
class GameState;
class InventoryItem;
struct ShopStats;
}

namespace Logging {
class Logger;
}
namespace Gui {

class IDialogScene;
class IGuiManager;

class Temple : public IDialogScene
{
    enum class State
    {
        Idle,
        Talk,
        Cure,
        Bless,
        BlessChosen,
        BlessAlreadyBlessed,
    };

public:
    Temple(
        BAK::GameState& gameState,
        IGuiManager& guiManager);

    Temple(const Temple&) = delete;
    Temple& operator=(const Temple&) = delete;

    void EnterTemple(
        BAK::KeyTarget keyTarget,
        unsigned templeIndex,
        BAK::ShopStats& shopStats,
        IDialogScene* parentScene);

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;

private:
    void StartDialog(BAK::KeyTarget keyTarget);
    void HandleBlessChoice(BAK::ChoiceIndex choice);
    void HandleUnblessChoice(BAK::ChoiceIndex choice);
    void HandleItemSelected(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>> selectedItem);

    State mState;
    BAK::GameState& mGameState;
    IGuiManager& mGuiManager;

    BAK::InventoryItem* mItem;
    BAK::ShopStats* mShopStats;
    BAK::KeyTarget mTarget;
    unsigned mTempleNumber;
    IDialogScene* mParentScene;

    const Logging::Logger& mLogger;
};

}
