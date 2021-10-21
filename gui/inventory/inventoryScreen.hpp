#pragma once

#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"
#include "bak/textureFactory.hpp"

#include "gui/inventory/inventorySlot.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/dragEndpoint.hpp"
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

template <typename Base>
using ItemEndpoint = DragEndpoint<
    Base,
    InventorySlot>;

class EquipmentSlot : public Widget
{
public:
    EquipmentSlot(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& mIcons,
        int icon)
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

    void PropagateUp(const DragEvent& event)
    {
        // Display the "blank" when you lift the equipment off the slot
        evaluate_if<DragStarted>(event, [&](const auto&){
                AddChildFront(&mBlank);
            });

        evaluate_if<DragEnded>(event, [&](const auto&){
                RemoveChild(&mBlank);
            });

        Widget::PropagateUp(event);
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


class InventoryScreen :
    public Widget
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
            glm::vec2{0, 0},
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
            [this]{ mGuiManager.ExitInventory(); },
            []{}
        },
        mGoldDisplay{
            mLayout.GetWidgetLocation(mGoldRequest),
            mLayout.GetWidgetDimensions(mGoldRequest),
        },
        mContainerTypeDisplay{
            [this](auto& item){ MoveItemToContainer(item); },
            mLayout.GetWidgetLocation(mContainerTypeRequest),
            mLayout.GetWidgetDimensions(mContainerTypeRequest),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            []{}, // Goto Keys, or goto Shop, or Goto Bag, or Goto Container...
            []{}
        },
        mWeapon{
            [this](auto& item){
                MoveItemToEquipmentSlot(item, BAK::ItemType::Sword); },
            glm::vec2{13, 15},
            glm::vec2{80, 29},
            mIcons,
            130
        },
        mCrossbow{
            [this](auto& item){
                MoveItemToEquipmentSlot(item, BAK::ItemType::Crossbow); },
            glm::vec2{13, 15 + 29},
            glm::vec2{80, 29},
            mIcons,
            130
        },
        mArmor{
            [this](auto& item){
                MoveItemToEquipmentSlot(item, BAK::ItemType::Armor); },
            glm::vec2{13, 15 + 29 * 2},
            glm::vec2{80, 58},
            mIcons,
            131
        },
        mInventoryItems{},
        mSelectedCharacter{0},
        mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
    {
        mCharacters.reserve(3);
    }

    void SetSelectedCharacter(unsigned character)
    {
        ClearChildren();

        mSelectedCharacter = character;
        UpdatePartyMembers();
        UpdateGold();
        UpdateInventoryContents();

        AddChildren();
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
            SetSelectedCharacter(mSelectedCharacter);
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

    auto& GetActiveCharacter(unsigned i)
    {
        return mGameState.GetParty()
            .GetActiveCharacter(i);
    }

    void TransferItem(InventorySlot& slot, unsigned character)
    {
        if (character != mSelectedCharacter)
        {
            auto item = slot.GetItem();

            if (GetActiveCharacter(character).GiveItem(item))
            {
                GetActiveCharacter(mSelectedCharacter)
                    .GetInventory()
                    .RemoveItem(slot.GetItemIndex());
            }
        }

        mLogger.Debug() << __FUNCTION__ << "Source: " << GetActiveCharacter(mSelectedCharacter).GetInventory() << "\n" << "Dest: " << GetActiveCharacter(character).GetInventory() << "\n";

        GetActiveCharacter(mSelectedCharacter).CheckPostConditions();
        GetActiveCharacter(character).CheckPostConditions();
        mNeedRefresh = true;
    }

    void MoveItemToEquipmentSlot(
        InventorySlot& item,
        BAK::ItemType slot)
    {
        mLogger.Debug() << "Move item to equipment slot: " 
            << item.GetItem() << " " << BAK::ToString(slot) << "\n";

        if (slot == BAK::ItemType::Sword)
        {
            if (GetActiveCharacter(mSelectedCharacter).IsSwordsman())
                GetActiveCharacter(mSelectedCharacter)
                    .ApplyItemToSlot(item.GetItemIndex(), slot);
            else
                GetActiveCharacter(mSelectedCharacter)
                    .ApplyItemToSlot(item.GetItemIndex(), BAK::ItemType::Staff);
        }

        GetActiveCharacter(mSelectedCharacter).CheckPostConditions();

        mNeedRefresh = true;
    }

    void MoveItemToContainer(InventorySlot& item)
    {
        mLogger.Debug() << "Move item to container: " << item.GetItem() << "\n";
        GetActiveCharacter(mSelectedCharacter).CheckPostConditions();
    }

    void UseItem(InventorySlot& item, BAK::InventoryIndex itemIndex)
    {
        auto& applyTo = GetActiveCharacter(mSelectedCharacter).GetInventory().GetAtIndex(itemIndex);
        mLogger.Debug() << "Use item : " << item.GetItem() << " with " << applyTo << "\n";
        GetActiveCharacter(mSelectedCharacter).CheckPostConditions();
    }

    void ShowItemDescription(const BAK::InventoryItem& item)
    {
        mGameState.SetDialogContext(item.mItemIndex.mValue);
        mGuiManager.StartDialog(
            BAK::DialogSources::GetItemDescription(),
            false,
            &mDialogScene);
    }

    void UpdatePartyMembers()
    {
        mCharacters.clear();

        const auto& party = mGameState.GetParty();
        for (unsigned person = 0; person < party.mActiveCharacters.size(); person++)
        {
            const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(party.mActiveCharacters[person]);
            mCharacters.emplace_back(
                [this, character=person](InventorySlot& slot){
                    TransferItem(slot, character);
                },
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

        const auto& character = GetActiveCharacter(mSelectedCharacter);
        const auto& inventory = character.GetInventory();

        std::vector<
            std::pair<
                BAK::InventoryIndex,
            const BAK::InventoryItem*>> items{};

        const auto numItems = inventory.GetItems().size();
        mInventoryItems.reserve(numItems);
        items.reserve(numItems);

        unsigned index{0};
        std::transform(
            inventory.GetItems().begin(), inventory.GetItems().end(),
            std::back_inserter(items),
            [&index](const auto& i) -> std::pair<BAK::InventoryIndex, const BAK::InventoryItem*> {
                return std::make_pair(BAK::InventoryIndex{index++}, &i);
            });

        std::sort(items.begin(), items.end(), [](const auto& l, const auto& r) 
        {
            return (std::get<1>(l)->GetObject().mImageSize 
                > std::get<1>(r)->GetObject().mImageSize);
            // FIXME: std::sort requires a strict weak ordering
            //        otherwise it fails.
                //|| (std::get<0>(l) < std::get<0>(r));
        });

        unsigned majorColumn = 0;
        unsigned minorColumn = 0;
        unsigned majorRow = 0;
        unsigned minorRow = 0;

        auto pos  = glm::vec2{105, 11};
        const auto slotDims = glm::vec2{40, 29};

        mCrossbow.ClearItem();
        mArmor.ClearItem();

        for (const auto& [invIndex, itemPtr] : items)
        {
            ASSERT(itemPtr);
            const auto& item = *itemPtr;
            const auto& [ss, ti, _] = mIcons.GetInventoryIcon(item.mItemIndex.mValue);
            const auto itemPos = pos + glm::vec2{
                    (majorColumn * 2 + minorColumn) * slotDims.x,
                    (majorRow * 2 + minorRow) * slotDims.y};

            auto dims = slotDims;

            if ((item.GetObject().mType == BAK::ItemType::Sword
                || item.GetObject().mType == BAK::ItemType::Staff)
                && item.IsEquipped())
            {
                auto scale = slotDims * glm::vec2{2, 1};
                if (item.GetObject().mType == BAK::ItemType::Staff)
                {
                    scale = scale * glm::vec2{1, 2};
                    mWeapon.SetDimensions({80, 58});
                }
                else
                {
                    mWeapon.SetDimensions({80, 29});
                }

                mWeapon.AddItem(
                    glm::vec2{0},
                    scale,
                    mFont,
                    mIcons,
                    invIndex,
                    item,
                    [&]{
                        ShowItemDescription(item);
                    });

                continue;
            }
            if (item.GetObject().mType == BAK::ItemType::Crossbow
                && item.IsEquipped())
            {
                mCrossbow.AddItem(
                    glm::vec2{0},
                    slotDims * glm::vec2{2, 1},
                    mFont,
                    mIcons,
                    invIndex,
                    item,
                    [&]{
                        ShowItemDescription(item);
                    });

                continue;
            }

            if (item.GetObject().mType == BAK::ItemType::Armor
                && item.IsEquipped())
            {
                mArmor.AddItem(
                    glm::vec2{0},
                    slotDims * glm::vec2{2},
                    mFont,
                    mIcons,
                    invIndex,
                    item,
                    [&]{
                        ShowItemDescription(item);
                    });

                continue;
            }

            mLogger.Spam() << "Item: " << item 
                << " mc: " << minorColumn << " MC: " << majorColumn 
                << " mr: " << minorRow << " MR: " << majorRow << "\n";
            if (item.GetObject().mImageSize == 1)
            {
                minorColumn += 1;
            }
            else if (item.GetObject().mImageSize == 2)
            {
                minorRow += 1;
                dims.x *= 2;
            }
            else if (item.GetObject().mImageSize == 4)
            {
                dims.x *= 2;
                dims.y *= 2;
                majorRow += 1;
            }

            mLogger.Spam() << "AfterPlace: " << item 
                << " mc: " << minorColumn << " MC: " << majorColumn 
                << " mr: " << minorRow << " MR: " << majorRow << "\n";
            mInventoryItems.emplace_back(
                [this, index=invIndex](auto& item){
                    UseItem(item, BAK::InventoryIndex{index}); },
                itemPos,
                dims,
                mFont,
                mIcons,
                invIndex,
                item,
                [&]{
                    ShowItemDescription(item);
                });

            if (minorColumn != 0 && minorColumn % 2 == 0)
            {
                minorColumn = 0;
                minorRow += 1;
            }

            if (minorRow != 0 && minorRow % 2 == 0)
            {
                minorRow = 0;
                majorRow += 1;
            }

            if (majorRow != 0 && majorRow % 2 == 0)
            {
                majorColumn += 1;
                majorRow = 0;
            }

            // Handle the final column
            if (majorColumn == 2 && minorColumn > 0)
            {
                minorRow += 1;
                minorColumn = 0;
            }

            mLogger.Spam() << "CorrectRows: " << item 
                << " mc: " << minorColumn << " MC: " << majorColumn 
                << " mr: " << minorRow << " MR: " << majorRow << "\n";
        }
    }

    void AddChildren()
    {
        AddChildBack(&mFrame);
        AddChildBack(&mExit);
        AddChildBack(&mGoldDisplay);

        SetContainerTypeImage();
        AddChildBack(&mContainerTypeDisplay);

        for (auto& character : mCharacters)
        {
            AddChildBack(&character);
        }

        AddChildBack(&mWeapon);

        if (GetActiveCharacter(mSelectedCharacter).IsSwordsman())
            AddChildBack(&mCrossbow);

        AddChildBack(&mArmor);

        for (auto& item : mInventoryItems)
        {
            AddChildBack(&item);
        }
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    std::vector<ItemEndpoint<ClickButtonImage>> mCharacters;
    ClickButtonImage mExit;
    TextBox mGoldDisplay;
    // click into shop or keys, etc.
    ItemEndpoint<ClickButtonImage> mContainerTypeDisplay;

    using ItemEndpointEquipmentSlot = ItemEndpoint<EquipmentSlot>;

    ItemEndpointEquipmentSlot mWeapon;
    ItemEndpointEquipmentSlot mCrossbow;
    ItemEndpointEquipmentSlot mArmor;
    std::vector<ItemEndpoint<InventorySlot>> mInventoryItems;

    unsigned mSelectedCharacter;
    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
