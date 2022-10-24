#pragma once

#include "bak/hotspot.hpp"
#include "bak/dialogSources.hpp"
#include "bak/money.hpp"
#include "bak/temple.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/sprites.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include "gui/dialogDisplay.hpp"
#include "gui/hotspot.hpp"
#include "gui/staticTTM.hpp"
#include "gui/widget.hpp"

namespace Gui {

class Temple : public IDialogScene
{
    enum class State
    {
        Idle,
        Talk,
        Cure,
        Bless
    };

public:
    Temple(
        BAK::GameState& gameState,
        IGuiManager& guiManager)
    :
        mState{State::Idle},
        mGameState{gameState},
        mGuiManager{guiManager},
        mShopStats{nullptr},
        mLogger{Logging::LogState::GetLogger("Gui::Temple")}
    {}
    
    Temple(const Temple&) = delete;
    Temple& operator=(const Temple&) = delete;

    void EnterTemple(
        BAK::KeyTarget keyTarget,
        unsigned templeIndex,
        BAK::ShopStats& shopStats)
    {
        mShopStats = &shopStats;
        mState = State::Idle;
        mGameState.SetDialogContext(templeIndex);
        mGuiManager.StartDialog(keyTarget, false, false, this);
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override
    {
        if (mState == State::Idle)
        {
            if (choice)
            { 
                if (*choice == BAK::ChoiceIndex{269})
                {
                }
                else if (*choice == BAK::ChoiceIndex{272})
                {
                }
                else if (*choice == BAK::ChoiceIndex{271})
                {
                    mGuiManager.SelectItem(
                        [this](auto charIndex, auto item){
                            HandleItemSelected(charIndex, item); });
                }
                else if (*choice == BAK::ChoiceIndex{268})
                {
                }
            }
        }
    }

    void DisplayNPCBackground() override { }
    void DisplayPlayerBackground() override { }

private:
    void HandleItemSelected(BAK::ActiveCharIndex charIndex, BAK::InventoryIndex itemIndex)
    {
        ASSERT(mShopStats);

        auto& character = mGameState.GetParty().GetCharacter(charIndex);
        auto& item = character.GetInventory().GetAtIndex(itemIndex);

        mGameState.SetActiveCharacter(character.GetIndex());
        mGameState.SetInventoryItem(item);
        mGameState.SetItemValue(BAK::Temple::CalculateBlessPrice(item, *mShopStats));

        if (BAK::Temple::IsBlessed(item))
        {
            mGuiManager.StartDialog(BAK::DialogSources::mBlessDialogItemAlreadyBlessed, false, false, this);
        }
        else if (!BAK::Temple::CanBlessItem(item))
        {
            mGuiManager.StartDialog(BAK::DialogSources::mBlessDialogCantBlessItem, false, false, this);
        //    BAK::Temple::BlessItem(item, shopStats);

        }
        else
        {
            mGuiManager.StartDialog(BAK::DialogSources::mBlessDialogCost, false, false, this);
        }
    }

    State mState;
    BAK::GameState& mGameState;
    IGuiManager& mGuiManager;

    BAK::ShopStats* mShopStats;

    const Logging::Logger& mLogger;
};

}
