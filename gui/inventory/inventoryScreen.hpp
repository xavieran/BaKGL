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
#include "gui/clickable.hpp"
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
        BAK::GameState& gameState);

    void SetSelectedCharacter(
        BAK::ActiveCharIndex character);

    void ClearContainer();
    void SetContainer(BAK::IContainer* container);

    /* Widget */
    bool OnMouseEvent(const MouseEvent& event) override;
    void PropagateUp(const DragEvent& event) override;

private:
    auto& GetCharacter(BAK::ActiveCharIndex i)
    {
        return mGameState.GetParty().GetCharacter(i);
    }

    void StartDialog(BAK::Target target)
    {
        mGuiManager.StartDialog(
            target,
            false,
            false,
            &mDialogScene);
    }

    void RefreshGui();

    void SetContainerTypeImage(unsigned containerType);
    void ShowContainer();
    void ShowCharacter(BAK::ActiveCharIndex character);

    void TransferItemFromCharacterToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex source,
        BAK::ActiveCharIndex dest);

    void TransferItemFromContainerToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex character);

    void SellItem(
        InventorySlot& slot,
        BAK::ActiveCharIndex character);

    void BuyItem(
        InventorySlot& slot,
        BAK::ActiveCharIndex character);

    void TransferItemToShop(
        InventorySlot& slot,
        BAK::ActiveCharIndex character);
    
    void TransferItemFromShopToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex character);
    
    void TransferItemToCharacter(
        InventorySlot& slot,
        BAK::ActiveCharIndex character);
    
    void MoveItemToEquipmentSlot(
        InventorySlot& item,
        BAK::ItemType slot);

    void MoveItemToContainer(InventorySlot& slot);
    
    void UseItem(InventorySlot& item, BAK::InventoryIndex itemIndex);
    
    void AdvanceNextPage();
    
    void ShowItemDescription(const BAK::InventoryItem& item);

    void HighlightValidDrops(const InventorySlot& slot);
    void UnhighlightDrops();
    
    void UpdatePartyMembers();
    void UpdateGold();
    void UpdateInventoryContents();
    
    void AddChildren();
    
    void CheckExclusivity();
    
private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    using CharacterButton = Clickable<
        Clickable<
            Widget,
            RightMousePress,
            std::function<void()>>,
        LeftMousePress,
        std::function<void()>>;

    std::vector<ItemEndpoint<CharacterButton>> mCharacters;
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
