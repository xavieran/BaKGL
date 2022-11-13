#pragma once

#include "audio/audio.hpp"

#include "bak/hotspot.hpp"
#include "bak/dialogSources.hpp"
#include "bak/money.hpp"
#include "bak/shop.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/sprites.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include "gui/dialogDisplay.hpp"
#include "gui/hotspot.hpp"
#include "gui/staticTTM.hpp"
#include "gui/widget.hpp"

namespace Gui {

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
        IGuiManager& guiManager)
    :
        mState{State::Idle},
        mGameState{gameState},
        mGuiManager{guiManager},
        mItem{nullptr},
        mShopStats{nullptr},
        mLogger{Logging::LogState::GetLogger("Gui::Repair")}
    {}
    
    Repair(const Repair&) = delete;
    Repair& operator=(const Repair&) = delete;

    void EnterRepair(
        BAK::ShopStats& shopStats)
    {
        mShopStats = &shopStats;
        mState = State::Idle;
        mGuiManager.SelectItem(
            [this](auto item){ HandleItemSelected(item); });
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override
    {
        mLogger.Info() << " Choice: " << choice << "\n";
        if (mState == State::Idle)
        {
        }
        else if (mState == State::Repairing)
        {
            mState = State::Idle;
            ASSERT(mItem);
            ASSERT(mShopStats);
            if (choice && *choice == BAK::ChoiceIndex{260})
            {
                mGameState.GetParty().LoseMoney(
                    BAK::Shop::CalculateRepairCost(*mItem, *mShopStats));
                BAK::Shop::RepairItem(*mItem);
            }
        }
    }

    void DisplayNPCBackground() override { }
    void DisplayPlayerBackground() override { }

private:
    void StartDialog(BAK::KeyTarget keyTarget)
    {
        mGuiManager.StartDialog(keyTarget, false, false, this);
    }

    void HandleItemSelected(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>> selectedItem)
    {
        ASSERT(mShopStats);

        if (!selectedItem)
        {
            mState = State::Idle;
            return;
        }

        const auto [charIndex, itemIndex] = *selectedItem;

        auto& character = mGameState.GetParty().GetCharacter(charIndex);
        auto& item = character.GetInventory().GetAtIndex(itemIndex);
        mItem = &item;

        mGameState.SetActiveCharacter(character.GetIndex());
        mGameState.SetInventoryItem(item);

        if (!BAK::Shop::CanRepair(item, *mShopStats))
        {
            StartDialog(BAK::DialogSources::mRepairShopCantRepairItem);
        }
        else if (!item.IsRepairable())
        {
            StartDialog(BAK::DialogSources::mRepairShopItemDoesntNeedRepair);
        }
        else
        {
            mState = State::Repairing;
            mGameState.SetItemValue(BAK::Shop::CalculateRepairCost(item, *mShopStats));
            StartDialog(BAK::DialogSources::mRepairShopCost);
        }

    }

    State mState;
    BAK::GameState& mGameState;
    IGuiManager& mGuiManager;

    BAK::InventoryItem* mItem;
    BAK::ShopStats* mShopStats;

    const Logging::Logger& mLogger;
};

}
