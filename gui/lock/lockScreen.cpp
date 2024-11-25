#include "gui/lock/lockScreen.hpp"

#include "audio/audio.hpp"

#include "bak/IContainer.hpp"
#include "bak/character.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/inventory.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/layout.hpp"
#include "bak/lock.hpp"
#include "bak/money.hpp"
#include "bak/state/lock.hpp"

#include "gui/animator.hpp"

#include "gui/lock/lock.hpp"

#include "gui/inventory/containerDisplay.hpp"
#include "gui/inventory/details.hpp"
#include "gui/inventory/inventorySlot.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/core/clickable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

LockScreen::LockScreen(
    IGuiManager& guiManager,
    const Backgrounds& backgrounds,
    const Icons& icons,
    const Font& font,
    BAK::GameState& gameState)
:
    // Black background
    Widget{
        RectTag{},
        glm::vec2{0, 0},
        glm::vec2{320, 200},
        Color::black,
        true
    },
    mGuiManager{guiManager},
    mFont{font},
    mIcons{icons},
    mGameState{gameState},
    mDialogScene{
        []{},
        []{},
        [](const auto&){}
    },
    mLayout{sLayoutFile},
    mFrame{
        ImageTag{},
        backgrounds.GetSpriteSheet(),
        backgrounds.GetScreen(sBackground),
        glm::vec2{0},
        GetPositionInfo().mDimensions,
        true
    },
    mCharacters{},
    mExit{
        mLayout.GetWidgetLocation(mExitRequest),
        mLayout.GetWidgetDimensions(mExitRequest),
        std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(mExitButton)),
        std::get<Graphics::TextureIndex>(mIcons.GetButton(mExitButton)),
        std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(mExitButton)),
        [this]{ mGuiManager.ExitLock(); },
        []{}
    },
    mGoldDisplay{
        mLayout.GetWidgetLocation(mGoldRequest),
        mLayout.GetWidgetDimensions(mGoldRequest),
    },
    mDetails{
        glm::vec2{},
        glm::vec2{},
        mIcons,
        mFont,
        [this]{ ExitDetails(); }
    },
    mDisplayDetails{},
    mLock{
        [this](){ ShowLockDescription(); },
        [this](const auto& item){ AttemptLock(item); },
        icons,
        glm::vec2{13, 12}},
    mContainerTypeDisplay{
        mLayout.GetWidgetLocation(mContainerTypeRequest),
        mLayout.GetWidgetDimensions(mContainerTypeRequest),
        std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryMiscIcon(11)),
        std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
        std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
        []{},
        []{}
    },
    mContainerScreen{
        {105, 11},
        {200, 121},
        mIcons,
        mFont,
        [this](const auto& item){
            ShowItemDescription(item);
        }
    },
    mSelectedCharacter{},
    mContainer{nullptr},
    mNeedRefresh{false},
    mUnlocked{false},
    mLogger{Logging::LogState::GetLogger("Gui::LockScreen")}
{
    mCharacters.reserve(3);
    mContainerTypeDisplay.CenterImage(
        std::get<2>(mIcons.GetInventoryMiscIcon(11)));
}

void LockScreen::SetSelectedCharacter(
    BAK::ActiveCharIndex character)
{
    mLogger.Debug() << "Setting seleted character to: " << character << "\n";
    mSelectedCharacter = character;
    mNeedRefresh = true;
}

void LockScreen::ResetUnlocked()
{
    mUnlocked = false;
}

bool LockScreen::IsUnlocked() const
{
    return mUnlocked;
}

BAK::IContainer* LockScreen::GetContainer() const
{
    return mContainer;
}

void LockScreen::SetContainer(BAK::IContainer* container)
{
    ASSERT(container != nullptr);

    mContainer = container;

    mLock.SetImageBasedOnLockType(
        BAK::ClassifyLock(container->GetLock().mRating));
    mLock.SetLocked();
    ResetUnlocked();

    // Automatically set to the highest skilled character
    const auto [character, _] = mGameState.GetPartySkill(
        BAK::SkillType::Lockpick,
        true);
    ASSERT(mGameState.GetParty().FindActiveCharacter(character));
    SetSelectedCharacter(
        *mGameState.GetParty().FindActiveCharacter(character));

    mContainerScreen.SetContainer(
        &mGameState.GetParty().GetKeys());

    RefreshGui();
}

/* Widget */
bool LockScreen::OnMouseEvent(const MouseEvent& event)
{
    const bool handled = Widget::OnMouseEvent(event);

    // Don't refresh things until we have finished
    // processing this event. This prevents deleting
    // children that are about to handle it.
    if (mNeedRefresh)
    {
        RefreshGui();
        mNeedRefresh = false;
    }

    return handled;
}

void LockScreen::PropagateUp(const DragEvent& event)
{
    mLogger.Debug() << __FUNCTION__ << " ev: " << event << "\n";
    bool handled = Widget::OnDragEvent(event);
    if (handled)
        return;
}

void LockScreen::RefreshGui()
{
    ClearChildren();

    UpdatePartyMembers();
    UpdateGold();

    mContainerScreen.RefreshGui();

    AddChildren();
}

BAK::Character& LockScreen::GetCharacter(BAK::ActiveCharIndex i)
{
    return mGameState.GetParty().GetCharacter(i);
}

void LockScreen::ShowLockDescription()
{
    unsigned context = 0;
    auto dialog = BAK::DialogSources::mLockDialog;

    const auto lockRating = mContainer->GetLock().mRating;
    const auto lockIndex = BAK::GetLockIndex(lockRating);
    const auto lockpickSkill = GetCharacter(*mSelectedCharacter)
        .GetSkill(BAK::SkillType::Lockpick);

    if (!lockIndex)
    {
        context = BAK::DescribeLock(lockpickSkill, lockRating);
    }
    else
    {
        if (BAK::State::CheckLockHasBeenSeen(mGameState, *lockIndex))
        {
            const auto item = BAK::GetCorrespondingKey(*lockIndex);
            if (mGameState.GetParty().HaveItem(item))
                context = 0;
            else
                context = 1;
            mGameState.SetInventoryItem(
                BAK::InventoryItemFactory::MakeItem(item, 1));
            dialog = BAK::DialogSources::mLockKnown;
        }
        else
        {
            context = BAK::DescribeLock(lockpickSkill, lockRating);
        }
    }

    mGameState.SetDialogContext_7530(context);
    mGuiManager.StartDialog(
        dialog,
        false,
        false,
        &mDialogScene);
}

void LockScreen::AttemptLock(const InventorySlot& itemSlot)
{
    ASSERT(mContainer);
    ASSERT(mSelectedCharacter);

    auto context = 0;
    auto dialog = BAK::KeyTarget{0};
    mGameState.SetActiveCharacter(GetCharacter(*mSelectedCharacter).mCharacterIndex);

    const auto& item = itemSlot.GetItem();
    ASSERT(item.IsKey());
    const auto& skill = GetCharacter(*mSelectedCharacter)
        .GetSkill(BAK::SkillType::Lockpick);
    const auto lockRating = mContainer->GetLock().mRating;

    if (item.GetItemIndex() == BAK::sPicklock)
    {
        if (BAK::CanPickLock(skill, lockRating))
        {
            AudioA::AudioManagerProvider::Get().GetAudioManager().PlaySound(sPickedLockSound);
            GetCharacter(*mSelectedCharacter)
                .ImproveSkill(BAK::SkillType::Lockpick, BAK::SkillChange::ExercisedSkill, 2);
            mGuiManager.AddAnimator(
                std::make_unique<LinearAnimator>(
                    .25,
                    glm::vec4{13, 17, 0, 0},
                    glm::vec4{13, 2, 0, 0},
                    [&](const auto& delta){
                        mLock.AdjustShacklePosition(glm::vec2{delta});
                        return false;
                    },
                    [&](){ Unlocked(BAK::DialogSources::mLockPicked); }
                ));

            mUnlocked = true;
        }
        else
        {
            if (BAK::PicklockSkillImproved())
            {
                GetCharacter(*mSelectedCharacter).ImproveSkill(
                    BAK::SkillType::Lockpick,
                    BAK::SkillChange::ExercisedSkill,
                    2);
            }

            if (BAK::PicklockBroken(skill, lockRating))
            {
                // Remove a picklock from inventory...
                dialog = BAK::DialogSources::mPicklockBroken;
                mGameState.GetParty().RemoveItem('P', 1);
                AudioA::AudioManagerProvider::Get().GetAudioManager().PlaySound(sPickBrokeSound);
            }
            else
            {
                dialog = BAK::DialogSources::mFailedToPickLock;
            }
        }
    }
    else
    {
        AudioA::AudioManagerProvider::Get().GetAudioManager().PlaySound(sUseKeySound);
        if (BAK::TryOpenLockWithKey(item, lockRating))
        {
            AudioA::AudioManagerProvider::Get().GetAudioManager().PlaySound(sOpenLockSound);
            // succeeded..
            ASSERT(BAK::GetLockIndex(lockRating));
            mGameState.Apply(BAK::State::SetLockHasBeenSeen, *BAK::GetLockIndex(lockRating));
            //mLock.SetUnlocked();
            mGuiManager.AddAnimator(
                std::make_unique<LinearAnimator>(
                    .2,
                    glm::vec4{13, 17, 0, 0},
                    glm::vec4{13, 2, 0, 0},
                    [&](const auto& delta){
                        mLock.AdjustShacklePosition(glm::vec2{delta});
                        return false;
                    },
                    [&](){ Unlocked(BAK::DialogSources::mKeyOpenedLock); }
                ));
            mUnlocked = true;
        }
        else
        {
            if (BAK::WouldKeyBreak(item, lockRating)
                && BAK::KeyBroken(item, skill, lockRating))
            {
                mGameState.GetParty().RemoveItem(item.GetItemIndex().mValue, 1);
                dialog = BAK::DialogSources::mKeyBroken;
                AudioA::AudioManagerProvider::Get().GetAudioManager().PlaySound(sKeyBrokeSound);
            }
            else
            {
                dialog = BAK::DialogSources::mKeyDoesntFit;
            }
        }
    }

    mNeedRefresh = true;

    if (!mUnlocked)
    {
        mGameState.SetDialogContext_7530(context);
        mGuiManager.StartDialog(
            dialog,
            false,
            false,
            &mDialogScene);
    }
}

void LockScreen::Unlocked(BAK::KeyTarget dialog)
{
    mGameState.SetDialogContext_7530(0);
    mDialogScene.SetDialogFinished(
        [this](const auto&)
        {
            mGuiManager.ExitLock();
            mDialogScene.ResetDialogFinished();
        });

    mGuiManager.StartDialog(
        dialog,
        false,
        false,
        &mDialogScene);
}

void LockScreen::ShowItemDescription(const BAK::InventoryItem& item)
{
    mDetails.AddItem(item, mGameState);
    mDisplayDetails = true;
    AddChildren();
}

void LockScreen::ExitDetails()
{
    mDisplayDetails = false;
    AddChildren();
}

void LockScreen::UpdatePartyMembers()
{
    mCharacters.clear();

    const auto& party = mGameState.GetParty();
    BAK::ActiveCharIndex person{0};
    do
    {
        const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(
            party.GetCharacter(person).GetIndex().mValue);
        mCharacters.emplace_back(
            [this, character=person]{
                // Switch character
                SetSelectedCharacter(character);
            },
            [this, character=person]{
                mGuiManager.ShowCharacterPortrait(character);
            },
            ImageTag{},
            spriteSheet,
            image,
            mLayout.GetWidgetLocation(person.mValue),
            mLayout.GetWidgetDimensions(person.mValue),
            true
        );

        if (person != mSelectedCharacter)
        {
            mCharacters[person.mValue].SetColor(glm::vec4{.05, .05, .05, 1}); 
            mCharacters[person.mValue].SetColorMode(Graphics::ColorMode::TintColor);
        }

        person = party.NextActiveCharacter(person);
    } while (person != BAK::ActiveCharIndex{0});
}

void LockScreen::UpdateGold()
{
    const auto gold = mGameState.GetParty().GetGold();
    const auto text = ToString(gold);
    const auto [textDims, _] = mGoldDisplay.SetText(mFont, text);

    // Justify text to the right
    const auto basePos = mLayout.GetWidgetLocation(mGoldRequest);
    const auto newPos = basePos 
        + glm::vec2{
            3 + mLayout.GetWidgetDimensions(mGoldRequest).x - textDims.x,
            4};

    mGoldDisplay.SetPosition(newPos);
}

void LockScreen::AddChildren()
{
    ClearChildren();

    AddChildBack(&mFrame);

    if (mDisplayDetails)
        AddChildBack(&mDetails);


    AddChildBack(&mExit);
    AddChildBack(&mGoldDisplay);

    AddChildBack(&mContainerTypeDisplay);

    for (auto& character : mCharacters)
        AddChildBack(&character);

    if (mDisplayDetails)
        return;

    AddChildBack(&mLock);
    AddChildBack(&mContainerScreen);
}

}
