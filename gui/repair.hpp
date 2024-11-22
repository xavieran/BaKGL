#pragma once

#include "bak/hotspot.hpp"
#include "bak/shop.hpp"

#include "gui/IDialogScene.hpp"

namespace Logging {
class Logger;
}

namespace Gui {

class IGuiManager;

class Repair : public IDialogScene
{
    enum class State
    {
        Idle,
        Repairing
    };

public:
    Repair(
        BAK::GameState& gameState,
        IGuiManager& guiManager);
    
    Repair(const Repair&) = delete;
    Repair& operator=(const Repair&) = delete;

    void EnterRepair(
        BAK::ShopStats& shopStats);
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;

private:
    void StartDialog(BAK::KeyTarget keyTarget);

    void HandleItemSelected(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>> selectedItem);

    State mState;
    BAK::GameState& mGameState;
    IGuiManager& mGuiManager;

    BAK::InventoryItem* mItem;
    BAK::ShopStats* mShopStats;

    const Logging::Logger& mLogger;
};

}
