#pragma once

#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"
#include "bak/textureFactory.hpp"

#include "gui/inventory/containerDisplay.hpp"
#include "gui/inventory/equipmentSlot.hpp"
#include "gui/inventory/inventorySlot.hpp"
#include "gui/inventory/itemArranger.hpp"
#include "gui/inventory/shopDisplay.hpp"

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

class InventoryScreen :
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
        mNextPage{
            mLayout.GetWidgetLocation(mNextPageRequest),
            mLayout.GetWidgetDimensions(mNextPageRequest),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(mNextPageButton)),
            std::get<Graphics::TextureIndex>(mIcons.GetButton(mNextPageButton)),
            std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(mNextPageButton)),
            [this]{ AdvanceNextPage(); },
            []{}
        },
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
            Graphics::SpriteSheetIndex{0},
            Graphics::TextureIndex{0},
            Graphics::TextureIndex{0},
            [&]{
                ShowContainer();
                RefreshGui();
            },
            []{}
        },
        mContainerScreen{
            {11, 11},
            {294, 121},
            mIcons,
            mFont,
            [this](const auto& item){
                ShowItemDescription(item);
            }
        },
        mShopScreen{
            {11, 11},
            {294, 121},
            mIcons,
            mFont,
            [this](const auto& item){
                ShowItemDescription(item);
            }
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
        mSelectedCharacter{},
        mDisplayContainer{false},
        mContainer{nullptr},
        mNeedRefresh{false},
        mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
    {
        mCharacters.reserve(3);
        ClearContainer();
    }

    void SetSelectedCharacter(
        BAK::ActiveCharIndex character)
    {
        ShowCharacter(character);
        RefreshGui();
    }

    void ClearContainer()
    {
        SetContainerTypeImage(11);

        mSelectedCharacter.reset();
        mContainerScreen.SetContainer(&mGameState.GetParty().GetKeys());
        mContainer = nullptr;
        mDisplayContainer = false;
    }

    void SetContainer(BAK::IContainer* container)
    {
        ASSERT(container != nullptr);

        mContainer = container;

        if (container->IsShop())
        {
            SetContainerTypeImage(7);
            mShopScreen.SetContainer(container);
        }
        else
        {
            SetContainerTypeImage(0);
            mContainerScreen.SetContainer(container);
        }

        ShowContainer();
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

        if (mDisplayContainer)
        {
            if (mContainer && mContainer->IsShop())
            {
                mShopScreen.RefreshGui();
            }
            else
            {
                mContainerScreen.RefreshGui();
            }
        }
        else
            UpdateInventoryContents();

        AddChildren();
    }

    auto& GetCharacter(BAK::ActiveCharIndex i)
    {
        return mGameState.GetParty().GetCharacter(i);
    }

    void SetContainerTypeImage(unsigned containerType)
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryMiscIcon(containerType);
        mContainerTypeDisplay.SetTexture(ss, ti);
        mContainerTypeDisplay.CenterImage(dims);
    }

    void ShowContainer()
    {
        mDisplayContainer = true;
        mSelectedCharacter.reset();
    }

    void ShowCharacter(BAK::ActiveCharIndex character)
    {
        mDisplayContainer = false;
        mSelectedCharacter = character;
    }

    void TransferItemFromCharacterToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex source,
        BAK::ActiveCharIndex dest)
    {
        auto item = slot.GetItem();
        if (GetCharacter(dest).CanAddItem(item))
        {
            GetCharacter(dest).GiveItem(item);
            GetCharacter(source)
                .GetInventory()
                .RemoveItem(slot.GetItemIndex());
        }
        else
        {
            // Set source and dest cahracter indices here..
            mGameState.SetDialogContext(1);
            mGuiManager.StartDialog(
                BAK::DialogSources::mContainerHasNoRoomForItem,
                false,
                false,
                &mDialogScene);
        }

        mLogger.Debug() << __FUNCTION__ << "Source: " 
            << GetCharacter(source).GetInventory() 
            << "\n" << "Dest: " << GetCharacter(dest).GetInventory() << "\n";
        GetCharacter(source).CheckPostConditions();
    }

    void TransferItemFromContainerToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex character)
    {
        ASSERT(mContainer);

        auto item = slot.GetItem();

        if (item.IsMoney() || item.IsKey())
        {
            ASSERT(mDisplayContainer);
            mGameState.GetParty().AddItem(item);
            mContainer->GetInventory()
                .RemoveItem(slot.GetItemIndex());
        }
        else if (GetCharacter(character).GiveItem(item))
        {
            mContainer->GetInventory()
                .RemoveItem(slot.GetItemIndex());
        }
    }

    void SellItem(
        InventorySlot& slot,
        BAK::ActiveCharIndex character)
    {
        ASSERT(mContainer);
        mGameState.GetParty().GainMoney(
            mShopScreen.GetBuyPrice(slot.GetItem()));
        mContainer->GiveItem(slot.GetItem());
        GetCharacter(character).GetInventory()
            .RemoveItem(slot.GetItemIndex());
        mNeedRefresh = true;
    }

    void BuyItem(
        InventorySlot& slot,
        BAK::ActiveCharIndex character)
    {
        ASSERT(mContainer);

        auto item = slot.GetItem();
        const auto price = mShopScreen.GetSellPrice(slot.GetItemIndex());
        if (mGameState.GetParty().GetGold().mValue >= price.mValue)
        {
            ASSERT(GetCharacter(character).CanAddItem(item));
            if (item.IsKey())
            {
                ASSERT(mDisplayContainer);
                mGameState.GetParty().AddItem(item);
            }
            else
            {
                ASSERT(GetCharacter(character).GiveItem(item));
            }
            mGameState.GetParty().LoseMoney(price);
        }

        mNeedRefresh = true;
    }

    void TransferItemToShop(
        InventorySlot& slot,
        BAK::ActiveCharIndex character)
    {
        ASSERT(mContainer);
        if (true) // mShopScreen.CanBuyItem(slot.GetItem());
        {
            mGameState.SetItemValue(mShopScreen.GetBuyPrice(slot.GetItem()));
            mGuiManager.StartDialog(
                BAK::DialogSources::mSellItemDialog,
                false,
                false,
                &mDialogScene);

            mDialogScene.SetDialogFinished(
                [this, &slot, character](const auto& choice)
                {
                    ASSERT(choice);
                    if (choice->mValue == BAK::Keywords::sYesIndex)
                    {
                        SellItem(slot, character);
                    }
                    mDialogScene.ResetDialogFinished();
                });
        }
        else
        {
            mGameState.SetDialogContext(0xb);
            mGuiManager.StartDialog(
                BAK::DialogSources::mContainerHasNoRoomForItem,
                false,
                false,
                &mDialogScene);
        }
    }

    void TransferItemFromShopToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex character)
    {
        ASSERT(mContainer);
        if (GetCharacter(character).CanAddItem(slot.GetItem()))
        {
            mGameState.SetItemValue(mShopScreen.GetSellPrice(slot.GetItemIndex()));
            mGuiManager.StartDialog(
                BAK::DialogSources::mBuyItemDialog,
                false,
                false,
                &mDialogScene);

            mDialogScene.SetDialogFinished(
                [this, &slot, character](const auto& choice)
                {
                    ASSERT(choice);
                    // FIXME: Add haggling...
                    if (choice->mValue == 0x104)
                    {
                        BuyItem(slot, character);
                    }
                    mDialogScene.ResetDialogFinished();
                });
        }
        else
        {
            mGameState.SetDialogContext(0xb);
            mGuiManager.StartDialog(
                BAK::DialogSources::mContainerHasNoRoomForItem,
                false,
                false,
                &mDialogScene);
        }
    }

    void TransferItemToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex character)
    {
        CheckExclusivity();

        // When displaying keys, can't transfer items
        if (mDisplayContainer && mContainer == nullptr)
            return;

        if (mSelectedCharacter && (*mSelectedCharacter != character))
        {
            TransferItemFromCharacterToCharacter(slot, *mSelectedCharacter, character);
        }
        else
        {
            if (mContainer->IsShop())
            {
                TransferItemFromShopToCharacter(slot, character);
            }
            else
            {
                TransferItemFromContainerToCharacter(slot, character);
            }
        }

        GetCharacter(character).CheckPostConditions();
        mNeedRefresh = true;
    }

    void MoveItemToEquipmentSlot(
        InventorySlot& item,
        BAK::ItemType slot)
    {
        ASSERT(mSelectedCharacter);

        mLogger.Debug() << "Move item to equipment slot: " 
            << item.GetItem() << " " << BAK::ToString(slot) << "\n";

        if (slot == BAK::ItemType::Sword)
        {
            if (GetCharacter(*mSelectedCharacter).IsSwordsman())
                GetCharacter(*mSelectedCharacter)
                    .ApplyItemToSlot(item.GetItemIndex(), slot);
            else
                GetCharacter(*mSelectedCharacter)
                    .ApplyItemToSlot(item.GetItemIndex(), BAK::ItemType::Staff);
        }
        else
        {
            GetCharacter(*mSelectedCharacter)
                .ApplyItemToSlot(item.GetItemIndex(), slot);
        }

        GetCharacter(*mSelectedCharacter).CheckPostConditions();

        mNeedRefresh = true;
    }

    void MoveItemToContainer(InventorySlot& slot)
    {
        // Can't move an item in a container to the container...
        if (mDisplayContainer)
            return;

        ASSERT(mSelectedCharacter);

        mLogger.Debug() << "Move item to container: " << slot.GetItem() << "\n";

        if (mContainer && mContainer->IsShop())
        {
            ASSERT(mSelectedCharacter);
            TransferItemToShop(slot, *mSelectedCharacter);
        }
        else if (mContainer)
        {
            mContainer->GetInventory().AddItem(slot.GetItem());
            GetCharacter(*mSelectedCharacter).GetInventory().RemoveItem(slot.GetItemIndex());
        }

        GetCharacter(*mSelectedCharacter).CheckPostConditions();
        mNeedRefresh = true;
    }

    void UseItem(InventorySlot& item, BAK::InventoryIndex itemIndex)
    {
        ASSERT(mSelectedCharacter);
        auto& applyTo = GetCharacter(*mSelectedCharacter).GetInventory().GetAtIndex(itemIndex);
        mLogger.Debug() << "Use item : " << item.GetItem() << " with " << applyTo << "\n";
        GetCharacter(*mSelectedCharacter).CheckPostConditions();
    }

    void AdvanceNextPage()
    {
        mLogger.Debug() << __FUNCTION__ << "\n";
        if (mDisplayContainer)
        {
            mShopScreen.AdvanceNextPage();
            mNeedRefresh = true;
        }
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
                [this, character=person](InventorySlot& slot){
                    TransferItemToCharacter(slot, character);
                },
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

    void UpdateInventoryContents()
    {
        CheckExclusivity();

        const auto& inventory = std::invoke([&]() -> const BAK::Inventory& {
            if (mDisplayContainer)
            {
                ASSERT(mContainer == nullptr);
                return mGameState.GetParty().GetKeys().GetInventory();
            }
            else
            {
                return GetCharacter(*mSelectedCharacter).GetInventory();
            }
        });


        mInventoryItems.clear();
        mInventoryItems.reserve(inventory.GetNumberItems());

        std::vector<
            std::pair<
                BAK::InventoryIndex,
            const BAK::InventoryItem*>> items{};

        const auto numItems = inventory.GetItems().size();
        items.reserve(numItems);

        unsigned index{0};
        std::transform(
            inventory.GetItems().begin(),
            inventory.GetItems().end(),
            std::back_inserter(items),
            [&index](const auto& i) -> std::pair<BAK::InventoryIndex, const BAK::InventoryItem*> {
                return std::make_pair(BAK::InventoryIndex{index++}, &i);
            });

        mCrossbow.ClearItem();
        mArmor.ClearItem();

        const auto slotDims = glm::vec2{40, 29};

        // Add equipped items
        for (const auto& [invIndex, itemPtr] : items)
        {
            ASSERT(itemPtr);
            const auto& item = *itemPtr;
            const auto& [ss, ti, _] = mIcons.GetInventoryIcon(item.mItemIndex.mValue);

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
        }

        // Don't display equipped items in the inventory
        items.erase(
            std::remove_if(
                items.begin(), items.end(),
                [&](const auto& i){
                    return inventory
                        .GetAtIndex(std::get<BAK::InventoryIndex>(i))
                        .IsEquipped();
            }),
            items.end());

        // Sort by item size to ensure nice packing
        std::sort(items.begin(), items.end(), [](const auto& l, const auto& r) 
        {
            return (std::get<1>(l)->GetObject().mImageSize 
                > std::get<1>(r)->GetObject().mImageSize);
        });

        const auto pos  = glm::vec2{105, 11};
        auto arranger = ItemArranger{};
        arranger.PlaceItems(
            items.begin(),
            items.end(),
            6,
            4,
            slotDims,
            false,
            [&](auto invIndex, const auto& item, const auto itemPos, const auto dims)
            {
                mInventoryItems.emplace_back(
                    [this, index=invIndex](auto& item){
                        UseItem(item, BAK::InventoryIndex{index}); },
                    itemPos + pos,
                    dims,
                    mFont,
                    mIcons,
                    invIndex,
                    item,
                    [&]{
                        ShowItemDescription(item);
                    });
            });
    }

    void AddChildren()
    {
        AddChildBack(&mFrame);
        AddChildBack(&mExit);
        AddChildBack(&mGoldDisplay);

        AddChildBack(&mContainerTypeDisplay);

        for (auto& character : mCharacters)
        {
            AddChildBack(&character);
        }

        if (mSelectedCharacter && !mDisplayContainer)
        {
            AddChildBack(&mWeapon);

            if (GetCharacter(*mSelectedCharacter).IsSwordsman())
                AddChildBack(&mCrossbow);

            AddChildBack(&mArmor);

            for (auto& item : mInventoryItems)
                AddChildBack(&item);
        }
        else if (mDisplayContainer)
        {
            if (mContainer && mContainer->IsShop())
            {
                AddChildBack(&mShopScreen);
                if (mShopScreen.GetMaxPages() > 1)
                    AddChildBack(&mNextPage);
            }
            else
            {
                AddChildBack(&mContainerScreen);
            }
        }
    }

    void CheckExclusivity()
    {
        ASSERT(bool{mSelectedCharacter} ^ mDisplayContainer);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    std::vector<ItemEndpoint<ClickButtonImage>> mCharacters;
    ClickButtonImage mNextPage;
    ClickButtonImage mExit;
    TextBox mGoldDisplay;
    // click into shop or keys, etc.
    ItemEndpoint<ClickButtonImage> mContainerTypeDisplay;

    ContainerDisplay mContainerScreen;
    ShopDisplay mShopScreen;

    using ItemEndpointEquipmentSlot = ItemEndpoint<EquipmentSlot>;

    ItemEndpointEquipmentSlot mWeapon;
    ItemEndpointEquipmentSlot mCrossbow;
    ItemEndpointEquipmentSlot mArmor;
    std::vector<ItemEndpoint<DraggableItem>> mInventoryItems;

    std::optional<BAK::ActiveCharIndex> mSelectedCharacter;
    bool mDisplayContainer;
    BAK::IContainer* mContainer;
    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
