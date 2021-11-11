#pragma once

#include "bak/lock.hpp"
#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"
#include "bak/textureFactory.hpp"

#include "gui/inventory/containerDisplay.hpp"
#include "gui/inventory/inventorySlot.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/dragEndpoint.hpp"
#include "gui/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class LockScreen :
    public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_INV.DAT";
    static constexpr auto sBackground = "INVENTOR.SCX";

    // Request offsets
    static constexpr auto mContainerTypeRequest = 3;

    static constexpr auto mNextPageButton = 52;
    static constexpr auto mNextPageRequest = 4;

    static constexpr auto mExitRequest = 5;
    static constexpr auto mExitButton = 13;

    static constexpr auto mGoldRequest = 6;

    LockScreen(
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
        mLock{
            [this](const auto& item){ AttemptLock(item); },
            glm::vec2{20, 64},
            std::get<glm::vec2>(mIcons.GetInventoryLockIcon(4)),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryLockIcon(4)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryLockIcon(4)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryLockIcon(4)),
            [&]{},
            [this]{ ShowLockDescription(); }
        },
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
        mLogger{Logging::LogState::GetLogger("Gui::LockScreen")}
    {
        mCharacters.reserve(3);
        mContainerTypeDisplay.CenterImage(
            std::get<2>(mIcons.GetInventoryMiscIcon(11)));
    }

    void SetSelectedCharacter(
        BAK::ActiveCharIndex character)
    {
        mSelectedCharacter = character;
        mNeedRefresh = true;
    }

    void SetImageBasedOnLockType(BAK::LockType lock)
    {
        const auto lockIndex = std::invoke([&](){
            switch (lock)
            {
            case BAK::LockType::Easy:   return 1;
            case BAK::LockType::Medium: return 2;
            case BAK::LockType::Hard:   return 3;
            case BAK::LockType::Unpickable: return 4;
            default: return 0;
            }
        });

        const auto [ss, ti, dims] = mIcons.GetInventoryLockIcon(lockIndex);
        mLock.SetTexture(ss, ti);
        mLock.SetDimensions(dims);
    }

    void SetContainer(BAK::IContainer* container)
    {
        ASSERT(container != nullptr);

        mContainer = container;

        SetImageBasedOnLockType(
            BAK::ClassifyLock(container->GetLockData().mRating));

        // Automatically set to the highest skilled character
        const auto [character, _] = mGameState.GetParty().GetSkill(BAK::SkillType::Lockpick, true);
        mSelectedCharacter = mGameState.GetParty().FindActiveCharacter(character);

        mContainerScreen.SetContainer(
            &mGameState.GetParty().GetKeys());

        RefreshGui();
    }

    /* Widget */
    bool OnMouseEvent(const MouseEvent& event) override
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

    void PropagateUp(const DragEvent& event) override
    {
        mLogger.Debug() << __FUNCTION__ << " ev: " << event << "\n";
        bool handled = Widget::OnDragEvent(event);
        if (handled)
            return;
    }

private:
    void RefreshGui()
    {
        ClearChildren();

        UpdatePartyMembers();
        UpdateGold();

        mContainerScreen.RefreshGui();

        AddChildren();
    }

    auto& GetCharacter(BAK::ActiveCharIndex i)
    {
        return mGameState.GetParty().GetCharacter(i);
    }

    void ShowLockDescription()
    {
        unsigned context = 0;
        auto dialog = BAK::DialogSources::mLockDialog;

        const auto lockRating = mContainer->GetLockData().mRating;
        const auto lockIndex = BAK::GetLockIndex(lockRating);
        const auto lockpickSkill = GetCharacter(*mSelectedCharacter)
            .GetSkills().GetSkill(BAK::SkillType::Lockpick).mCurrent;

        if (!lockIndex)
        {
            context = BAK::DescribeLock(lockpickSkill, lockRating);
        }
        else
        {
            if (mGameState.CheckLockSeen(*lockIndex))
            {
                const auto item = BAK::GetCorrespondingKey(*lockIndex);
                if (mGameState.GetParty().HaveItem(item))
                    context = 0;
                else
                    context = 1;
                mGameState.SetKeyName(item);
                dialog = BAK::DialogSources::mLockKnown;
            }
            else
            {
                context = BAK::DescribeLock(lockpickSkill, lockRating);
            }
        }

        mGameState.SetDialogContext(context);
        mGuiManager.StartDialog(
            dialog,
            false,
            false,
            &mDialogScene);
    }

    void AttemptLock(const InventorySlot& itemSlot)
    {
        ASSERT(mContainer);
        ASSERT(mSelectedCharacter);

        auto context = 0;
        auto dialog = BAK::KeyTarget{0};

        const auto& item = itemSlot.GetItem();
        ASSERT(item.IsKey());
        const auto& skill = GetCharacter(*mSelectedCharacter)
            .GetSkills().GetSkill(BAK::SkillType::Lockpick);
        const auto lockRating = mContainer->GetLockData().mRating;

        if (item.mItemIndex == BAK::ItemIndex{'P'}) // Picklock
        {
            if (BAK::CanPickLock(skill, lockRating))
            {
                // locks are for blah and blah
                // success
                // ImproveSkill x2
                dialog = BAK::DialogSources::mLockPicked;
            }
            else
            {
                if (BAK::PicklockSkillImproved())
                {
                    //ImproveSkill x1
                }

                if (BAK::PicklockBroken(skill, lockRating))
                {
                    // Remove a picklock from inventory...
                    dialog = BAK::DialogSources::mPicklockBroken;
                }
                else
                {
                    dialog = BAK::DialogSources::mFailedToPickLock;
                }
            }
        }
        else
        {
            if (BAK::TryOpenLockWithKey(item, lockRating))
            {
                // succeeded..
                ASSERT(BAK::GetLockIndex(lockRating));
                mGameState.MarkLockSeen(*BAK::GetLockIndex(lockRating));
                dialog = BAK::DialogSources::mKeyOpenedLock;
            }
            else
            {
                if (BAK::WouldKeyBreak(item, lockRating)
                    && BAK::KeyBroken(item, skill, lockRating))
                {
                    // break key...
                    dialog = BAK::DialogSources::mKeyBroken;
                }
                else
                {
                    dialog = BAK::DialogSources::mKeyDoesntFit;
                }
            }
        }

        mGameState.SetDialogContext(context);
        mGuiManager.StartDialog(
            dialog,
            false,
            false,
            &mDialogScene);
    }

    void ShowItemDescription(const BAK::InventoryItem& item)
    {
        unsigned context = 0;
        auto dialog = BAK::KeyTarget{0};
        // FIXME: Probably want to put this logic elsewhere...
        if (item.GetObject().mType == BAK::ItemType::Scroll)
        {
            context = item.mCondition;
            dialog = BAK::DialogSources::GetScrollDescription();
        }
        else
        {
            context = item.mItemIndex.mValue;
            dialog = BAK::DialogSources::GetItemDescription();
        }

        mGameState.SetDialogContext(context);
        mGuiManager.StartDialog(
            dialog,
            false,
            false,
            &mDialogScene);
    }

    void UpdatePartyMembers()
    {
        mCharacters.clear();

        const auto& party = mGameState.GetParty();
        BAK::ActiveCharIndex person{0};
        do
        {
            const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(
                party.GetCharacter(person).GetIndex().mValue);
            mCharacters.emplace_back(
                mLayout.GetWidgetLocation(person.mValue),
                mLayout.GetWidgetDimensions(person.mValue),
                spriteSheet,
                image,
                image,
                [this, character=person]{
                    // Switch character
                    SetSelectedCharacter(character);
                },
                [this, character=person]{
                    mGuiManager.ShowCharacterPortrait(character);
                }
            );
            person = party.NextActiveCharacter(person);
        } while (person != BAK::ActiveCharIndex{0});
    }

    void UpdateGold()
    {
        const auto gold = mGameState.GetParty().GetGold();
        const auto text = ToString(gold);
        const auto [textDims, _] = mGoldDisplay.AddText(mFont, text);

        // Justify text to the right
        const auto basePos = mLayout.GetWidgetLocation(mGoldRequest);
        const auto newPos = basePos 
            + glm::vec2{
                3 + mLayout.GetWidgetDimensions(mGoldRequest).x - textDims.x,
                4};

        mGoldDisplay.SetPosition(newPos);
    }

    void AddChildren()
    {
        AddChildBack(&mFrame);
        AddChildBack(&mExit);
        AddChildBack(&mGoldDisplay);

        AddChildBack(&mContainerTypeDisplay);

        for (auto& character : mCharacters)
            AddChildBack(&character);

        AddChildBack(&mLock);
        AddChildBack(&mContainerScreen);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    std::vector<ClickButtonImage> mCharacters;
    ClickButtonImage mExit;
    TextBox mGoldDisplay;
    ItemEndpoint<ClickButtonImage> mLock;
    ClickButtonImage mContainerTypeDisplay;
    ContainerDisplay mContainerScreen;

    std::optional<BAK::ActiveCharIndex> mSelectedCharacter;
    BAK::IContainer* mContainer;
    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
