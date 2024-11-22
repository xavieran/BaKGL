#include "gui/repair.hpp"

#include "bak/hotspot.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/shop.hpp"

#include "com/ostream.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/dialogDisplay.hpp"

namespace Gui {

Repair::Repair(
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

void Repair::EnterRepair(
    BAK::ShopStats& shopStats)
{
    mShopStats = &shopStats;
    mState = State::Idle;
    mGuiManager.SelectItem(
        [this](auto item){ HandleItemSelected(item); });
}

void Repair::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
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
        const auto cost = BAK::Shop::CalculateRepairCost(*mItem, *mShopStats);
        if (choice && *choice == BAK::ChoiceIndex{260}
            && mGameState.GetParty().GetGold().mValue > cost.mValue)
        {
            mGameState.GetParty().LoseMoney(cost);
            BAK::Shop::RepairItem(*mItem);
        }
    }
}

void Repair::DisplayNPCBackground() {}
void Repair::DisplayPlayerBackground() { }

void Repair::StartDialog(BAK::KeyTarget keyTarget)
{
    mGuiManager.StartDialog(keyTarget, false, false, this);
}

void Repair::HandleItemSelected(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>> selectedItem)
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
    else if (!item.IsRepairableByShop() && !item.IsBroken())
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

}
