#pragma once

#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/portrait.hpp"
#include "gui/ratings.hpp"
#include "gui/skills.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {



class InventorySlot : public Widget
{
public:
    InventorySlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        const Icons& icons,
        const BAK::InventoryItem& item,
        std::function<void()>&& showItemDescription)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{},
            true
        },
        mItemRef{item},
        mShowItemDescription{std::move(showItemDescription)},
        mIsSelected{false},
        mQuantity{
            glm::vec2{0, 0},
            glm::vec2{40, 30}
        },
        mItem{
            ImageTag{},

            std::get<Graphics::SpriteSheetIndex>(
                icons.GetInventoryIcon(item.mObject.mImageIndex)),
            std::get<Graphics::TextureIndex>(
                icons.GetInventoryIcon(item.mObject.mImageIndex)),
            pos,
            std::get<glm::vec2>(
                icons.GetInventoryIcon(item.mObject.mImageIndex)),
            true
        }
    {
        assert(mShowItemDescription);
        mItem.SetCenter(GetCenter() - GetTopLeft());

        UpdateQuantity(font, item);

        AddChildren();
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const RightMousePress& p){ return RightMousePressed(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    bool LeftMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            mIsSelected = true;
            Logging::LogDebug("InventoryItem") << "Clicked: " << mItemRef << "\n"
                << mItemRef.mObject << "\n";
        }
        else
            mIsSelected = false;

        if (mIsSelected)
            SetColor(Color::itemHighlighted);
        else
            SetColor(glm::vec4{});
        
        return false;
    }

    bool RightMousePressed(glm::vec2 click)
    {
        if (Within(click))
        {
            mIsSelected = true;
            mShowItemDescription();
        }
        else
            mIsSelected = false;

        if (mIsSelected)
            SetColor(Color::itemHighlighted);
        else
            SetColor(glm::vec4{});

        return false;
    }

    void UpdateQuantity(
        const Font& font,
        const BAK::InventoryItem& item)
    {
        std::stringstream ss{};
        ss << "#" << +item.mCondition << "%";
        const auto& [textDims, _] = mQuantity.AddText(font, ss.str());
        const auto& dims = GetPositionInfo().mDimensions;
        mQuantity.SetPosition(dims 
            - textDims 
            + glm::vec2{4, 2});
    }

private:
    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mItem);
        AddChildBack(&mQuantity);
    }

    const BAK::InventoryItem& mItemRef;
    std::function<void()> mShowItemDescription;
    bool mIsSelected;

    TextBox mQuantity;
    Widget mItem;
};

class EquipmentSlot : public Widget
{
public:
    EquipmentSlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& mIcons,
        unsigned icon)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{},
            true
        },
        mBlank{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(
                mIcons.GetInventoryIcon(icon)),
            std::get<Graphics::TextureIndex>(
                mIcons.GetInventoryIcon(icon)),
            glm::vec2{0},
            dims,
            true
        }
    {
        ClearItem();
    }

    template <typename ...Args>
    void AddItem(Args&&... args)
    {
        ClearChildren();
        mItem.emplace(std::forward<Args>(args)...);
        AddChildBack(&(*mItem));
    }

    void ClearItem()
    {
        ClearChildren();
        AddChildBack(&mBlank);
    }

private:
    std::optional<InventorySlot> mItem;
    Widget mBlank;
};

class InventoryScreen : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_INV.DAT";
    static constexpr auto sBackground = "INVENTOR.SCX";

    // Request offsets
    static constexpr auto mContainerTypeRequest = 3;
    static constexpr auto mUseItemRequest = 4;
    static constexpr auto mExitRequest = 5;
    static constexpr auto mExitButton = 13;
    static constexpr auto mGoldRequest = 6;

    InventoryScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState)
    :
        // Black background
        Widget{
            RectTag{},
            glm::vec2{0},
            glm::vec2{320, 200},
            Color::black,
            true
        },
        mGuiManager{guiManager},
        mFont{font},
        mIcons{icons},
        mGameState{gameState},
        mDialogScene{},
        mLayout{sLayoutFile},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            GetPositionInfo().mPosition,
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
            [this]{ mGuiManager.ExitInventory(); },
            []{}
        },
        mGoldDisplay{
            mLayout.GetWidgetLocation(mGoldRequest),
            mLayout.GetWidgetDimensions(mGoldRequest),
        },
        mContainerTypeDisplay{
            mLayout.GetWidgetLocation(mContainerTypeRequest),
            mLayout.GetWidgetDimensions(mContainerTypeRequest),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            []{}, // Goto Keys, or goto Shop, or Goto Bag, or Goto Container...
            []{}
        },
        mArmor{
            glm::vec2{13, 15 + 29 * 2},
            glm::vec2{80, 58},
            mIcons,
            131
        },
        mInventoryItems{},
        mSelectedCharacter{0},
        mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
    {
        AddChildren();
    }

    void SetSelectedCharacter(unsigned character)
    {
        mSelectedCharacter = character;
        UpdatePartyMembers();
        UpdateGold();
        UpdateInventoryContents();

        AddChildren();
    }

private:
    void UpdatePartyMembers()
    {
        mCharacters.clear();
        mCharacters.reserve(3);

        const auto& party = mGameState.GetParty();
        for (unsigned person = 0; person < party.mActiveCharacters.size(); person++)
        {
            const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(party.mActiveCharacters[person]);
            mCharacters.emplace_back(
                mLayout.GetWidgetLocation(person),
                mLayout.GetWidgetDimensions(person),
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
        }
    }

    void UpdateGold()
    {
        const auto gold = mGameState.GetParty().GetGold();
        const auto sovereigns = BAK::GetSovereigns(gold);
        const auto royals = BAK::GetRemainingRoyals(gold);
        std::stringstream ss{};
        ss << "#" << sovereigns << "s " << royals << "r";
        const auto [textDims, _] = mGoldDisplay.AddText(mFont, ss.str());

        // Justify text to the right
        const auto basePos = mLayout.GetWidgetLocation(mGoldRequest);
        const auto newPos = basePos 
            + glm::vec2{
                3 + mLayout.GetWidgetDimensions(mGoldRequest).x - textDims.x,
                4};

        mGoldDisplay.SetPosition(newPos);
    }

    void SetContainerTypeImage()
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryMiscIcon(11);
        mContainerTypeDisplay.CenterImage(dims);
    }

    void UpdateInventoryContents()
    {
        mInventoryItems.clear();

        const auto& character = mGameState.GetParty().GetActiveCharacter(mSelectedCharacter);
        mLogger.Info() << "Updating Character: " << character << "\n";
        const auto& inventory = character.GetInventory();

        std::vector<const BAK::InventoryItem*> items{};

        const auto numItems = inventory.GetItems().size();
        mInventoryItems.reserve(numItems);
        items.reserve(numItems);

        std::transform(
            inventory.GetItems().begin(), inventory.GetItems().end(),
            std::back_inserter(items),
            [](const auto& i) -> const BAK::InventoryItem* {
                return &i;
            });

        std::sort(items.begin(), items.end(), [](const auto& l, const auto& r) 
        {
            return l->mObject.mImageSize > r->mObject.mImageSize;
        });

        unsigned majorColumn = 0;
        unsigned minorColumn = 0;
        unsigned majorRow = 0;
        unsigned minorRow = 0;

        auto pos  = glm::vec2{105, 11};
        const auto slotDims = glm::vec2{40, 29};

        for (const auto& itemPtr : items)
        {
            assert(itemPtr);
            const auto& item = *itemPtr;
            const auto& [ss, ti, _] = mIcons.GetInventoryIcon(item.mItemIndex.mValue);
            const auto itemPos = pos + glm::vec2{
                    (majorColumn * 2 + minorColumn) * slotDims.x,
                    (majorRow * 2 + minorRow) * slotDims.y};

            auto dims = slotDims;

            if (item.mObject.mType == BAK::ItemType::Armor
                && item.IsEquipped())
            {
                mArmor.AddItem(
                    glm::vec2{0},
                    slotDims * glm::vec2{2},
                    mFont,
                    mIcons,
                    item,
                    [&, itemIndex=item.mItemIndex](){
                        mGameState.SetDialogContext(itemIndex.mValue);
                        mGuiManager.StartDialog(
                            BAK::DialogSources::GetItemDescription(),
                            false,
                            &mDialogScene);
                    });

                continue;
            }

            if (item.mObject.mImageSize == 1)
            {
                minorColumn += 1;
            }
            else if (item.mObject.mImageSize == 2)
            {
                minorRow += 1;
                dims.x *= 2;
            }
            else if (item.mObject.mImageSize == 4)
            {
                dims.x *= 2;
                dims.y *= 2;
                majorRow += 1;
            }

            mInventoryItems.emplace_back(
                itemPos,
                dims,
                mFont,
                mIcons,
                item,
                [&, itemIndex=item.mItemIndex](){
                    mGameState.SetDialogContext(itemIndex.mValue);
                    mGuiManager.StartDialog(
                        BAK::DialogSources::GetItemDescription(),
                        false,
                        &mDialogScene);
                });

            if (minorColumn == 2)
            {
                minorColumn = 0;
                minorRow += 1;
            }

            if (minorRow == 2)
            {
                minorRow = 0;
                majorRow += 1;
            }

            if (majorRow == 2)
            {
                majorColumn += 1;
                majorRow = 0;
            }
        }
    }

    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);
        AddChildBack(&mExit);
        AddChildBack(&mGoldDisplay);

        SetContainerTypeImage();
        AddChildBack(&mContainerTypeDisplay);

        for (auto& character : mCharacters)
            AddChildBack(&character);

        AddChildBack(&mArmor);

        for (auto& item : mInventoryItems)
            AddChildBack(&item);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    std::vector<ClickButtonImage> mCharacters;
    ClickButtonImage mExit;
    TextBox mGoldDisplay;
    // click into shop or keys, etc.
    ClickButtonImage mContainerTypeDisplay;

    EquipmentSlot mArmor;
    std::vector<InventorySlot> mInventoryItems;

    unsigned mSelectedCharacter;
    const Logging::Logger& mLogger;
};

}
