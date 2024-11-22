#include "gui/temple/temple.hpp"

#include "audio/audio.hpp"

#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/temple.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"

#include "gui/dialogDisplay.hpp"

namespace Gui {

Temple::Temple(
    BAK::GameState& gameState,
    IGuiManager& guiManager)
:
    mState{State::Idle},
    mGameState{gameState},
    mGuiManager{guiManager},
    mItem{nullptr},
    mShopStats{nullptr},
    mTarget{BAK::KeyTarget{0}},
    mTempleNumber{0},
    mParentScene{},
    mLogger{Logging::LogState::GetLogger("Gui::Temple")}
{}

void Temple::EnterTemple(
    BAK::KeyTarget keyTarget,
    unsigned templeIndex,
    BAK::ShopStats& shopStats,
    IDialogScene* parentScene)
{
    mShopStats = &shopStats;
    mTarget = keyTarget;
    mState = State::Idle;
    mTempleNumber = templeIndex;
    mGameState.SetDialogContext_7530(templeIndex);
    mParentScene = parentScene;
    StartDialog(keyTarget);
}

void Temple::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    mLogger.Info() << " Choice: " << choice << "\n";
    if (mState == State::Idle)
    {
        if (choice)
        { 
            // (269, #Talk),(272, #Cure),(271, #Bless),(268, #Done)
            if (*choice == BAK::ChoiceIndex{269})
            {
                StartDialog(mTarget);
            }
            else if (*choice == BAK::ChoiceIndex{272})
            {
                if (mGameState.GetEndOfDialogState() != -1)
                {
                    bool canHeal = false;
                    for (unsigned i = 0; i < mGameState.GetParty().GetNumCharacters(); i++)
                    {
                        const auto& character = mGameState.GetParty().GetCharacter(BAK::ActiveCharIndex{i});
                        const auto cureCost = BAK::Temple::CalculateCureCost(
                            mShopStats->GetTempleHealFactor(),
                            mTempleNumber == BAK::Temple::sTempleOfSung,
                            character.mSkills,
                            character.GetConditions(),
                            character.GetSkillAffectors());
                        if (cureCost.mValue != 0)
                        {
                            // will this always be true?
                            // I vaguely remember temple of Eortis might be free
                            canHeal = true;
                        }
                    }
                    mState = State::Cure;
                    if (canHeal)
                    {
                        mGuiManager.ShowCureScreen(
                            mTempleNumber,
                            mShopStats->GetTempleHealFactor(),
                            [this](){
                                DialogFinished(std::nullopt); });
                    }
                    else
                    {
                        StartDialog(
                            mTempleNumber == BAK::Temple::sTempleOfSung
                                ? BAK::DialogSources::mHealDialogCantHealNotSick
                                : BAK::DialogSources::mHealDialogCantHealNotSickEnough);
                    }
                }
                else
                {
                    mState = State::Idle;
                    StartDialog(mTarget);
                }
            }
            else if (*choice == BAK::ChoiceIndex{271})
            {
                mLogger.Debug() << "End of dialog state: " << mGameState.GetEndOfDialogState() << "\n";
                if (mGameState.GetEndOfDialogState() != -1)
                {
                    mState = State::Bless;
                    mGuiManager.SelectItem(
                        [this](auto item){ HandleItemSelected(item); });
                }
                else
                {
                    mState = State::Idle;
                    StartDialog(mTarget);
                }
            }
            else if (*choice == BAK::ChoiceIndex{268})
            {
                mParentScene->DialogFinished(std::nullopt);
            }
        }
    }
    else if (mState == State::BlessChosen)
    {
        if (choice)
            HandleBlessChoice(*choice);
    }
    else if (mState == State::BlessAlreadyBlessed)
    {
        ASSERT(choice);
        HandleUnblessChoice(*choice);
    }
    else if (mState == State::Cure)
    {
        mState = State::Idle;
        StartDialog(mTarget);
    }
}

void Temple::DisplayNPCBackground()
{
    assert(mParentScene);
    mParentScene->DisplayNPCBackground();
}

void Temple::DisplayPlayerBackground()
{
    assert(mParentScene);
    mParentScene->DisplayPlayerBackground();
}

void Temple::StartDialog(BAK::KeyTarget keyTarget)
{
    mGuiManager.StartDialog(keyTarget, false, false, this);
}

void Temple::HandleBlessChoice(BAK::ChoiceIndex choice)
{
    ASSERT(mItem);
    const auto cost = BAK::Temple::CalculateBlessPrice(*mItem, *mShopStats);
    if (choice == BAK::ChoiceIndex{260} && mGameState.GetMoney() > cost)
    {
        mLogger.Info() << __FUNCTION__ << " Blessing item\n";
        mGameState.GetParty().LoseMoney(cost);
        if (BAK::Temple::IsBlessed(*mItem))
        {
            BAK::Temple::RemoveBlessing(*mItem);
        }
        BAK::Temple::BlessItem(*mItem, *mShopStats);
        AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{0x3e});
    }
}

void Temple::HandleUnblessChoice(BAK::ChoiceIndex choice)
{
    ASSERT(mItem);
    if (choice == BAK::ChoiceIndex{256})
    {
        mLogger.Info() << __FUNCTION__ << " Unblessing item\n";
        mState = State::BlessChosen;
        StartDialog(BAK::DialogSources::mBlessDialogCost);
    }
}

void Temple::HandleItemSelected(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>> selectedItem)
{
    ASSERT(mShopStats);

    if (!selectedItem)
    {
        mState = State::Idle;
        StartDialog(mTarget);
        return;
    }

    const auto [charIndex, itemIndex] = *selectedItem;

    auto& character = mGameState.GetParty().GetCharacter(charIndex);
    auto& item = character.GetInventory().GetAtIndex(itemIndex);
    mItem = &item;

    mGameState.SetActiveCharacter(character.GetIndex());
    mGameState.SetInventoryItem(item);
    mGameState.SetItemValue(BAK::Temple::CalculateBlessPrice(item, *mShopStats));

    if (BAK::Temple::IsBlessed(item))
    {
        mState = State::BlessAlreadyBlessed;
        StartDialog(BAK::DialogSources::mBlessDialogItemAlreadyBlessed);
    }
    else if (!BAK::Temple::CanBlessItem(item))
    {
        StartDialog(BAK::DialogSources::mBlessDialogCantBlessItem);
    }
    else
    {
        mState = State::BlessChosen;
        if (mItem->IsItemType(BAK::ItemType::Sword))
        {
            mGameState.SetDialogContext_7530(0);
        }
        else
        {
            mGameState.SetDialogContext_7530(1);
        }
        StartDialog(BAK::DialogSources::mBlessDialogCost);
    }
}

}
