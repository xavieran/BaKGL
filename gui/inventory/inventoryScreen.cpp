#include "gui/inventory/inventoryScreen.hpp"

#include "bak/haggle.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/itemInteractions.hpp"

namespace Gui {

InventoryScreen::InventoryScreen(
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
        [this]{
            mGuiManager.ExitInventory();
        },
        []{}
    },
    mGoldDisplay{
        mLayout.GetWidgetLocation(mGoldRequest),
        mLayout.GetWidgetDimensions(mGoldRequest),
    },
    mContainerTypeDisplay{
        [this](auto& item){ SplitStackBeforeMoveItemToContainer(item); },
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
    mDetails{
        glm::vec2{},
        glm::vec2{},
        mIcons,
        mFont,
        [this]{ ExitDetails(); }
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
    mSplitStackDialog{
        {128, 80},
        mFont
    },
    mSelectedCharacter{},
    mDisplayContainer{false},
    mItemSelectionMode{false},
    mItemSelectionCallback{nullptr},
    mSelectedItem{},
    mContainer{nullptr},
    mNeedRefresh{false},
    mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
{
    mCharacters.reserve(3);
    ClearContainer();
}

void InventoryScreen::SetSelectedCharacter(
    BAK::ActiveCharIndex character)
{
    mSelectedItem = std::nullopt;
    ShowCharacter(character);
    RefreshGui();
}

void InventoryScreen::ClearContainer()
{
    SetContainerTypeImage(11);

    mSelectedCharacter.reset();
    mContainerScreen.SetContainer(&mGameState.GetParty().GetKeys());
    mContainer = nullptr;
    mDisplayContainer = false;
}

void InventoryScreen::SetContainer(BAK::IContainer* container)
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
bool InventoryScreen::OnMouseEvent(const MouseEvent& event)
{
    const bool handled = Widget::OnMouseEvent(event);
    {
    }

    if (std::holds_alternative<LeftMousePress>(event)
        && mItemSelectionMode)
    {
        HandleItemSelected();
        mNeedRefresh = true;
    }

    // Don't refresh things until we have finished
    // processing this event. This prevents deleting
    // children that are about to handle it.
    if (mNeedRefresh 
        // dirty hack :(
        // probably want some callback to force a refresh instead of this...
        || mItemSelectionMode) 
    {
        RefreshGui();
        mNeedRefresh = false;
    }

    return handled;
}

void InventoryScreen::PropagateUp(const DragEvent& event)
{
    mLogger.Debug() << __FUNCTION__ << " ev: " << event << "\n";

    if (std::holds_alternative<DragStarted>(event))
    {
        auto& slot = *static_cast<InventorySlot*>(
                std::get<DragStarted>(event).mWidget);
        HighlightValidDrops(slot);
        // FIXME: Ideally this happens when an item is first clicked,
        // not just when dragged...
        mGameState.SetInventoryItem(slot.GetItem());
    }
    else if (std::holds_alternative<DragEnded>(event))
    {
        UnhighlightDrops();
        const auto& pos = std::get<DragEnded>(event).mValue;
        mSplitStackDialog.SetCenter(pos);
    }

    bool handled = Widget::OnDragEvent(event);
    if (handled)
        return;
}

void InventoryScreen::RefreshGui()
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

void InventoryScreen::ExitDetails()
{
    mDisplayDetails = false;
    mNeedRefresh = true;
}

void InventoryScreen::SetContainerTypeImage(unsigned containerType)
{
    const auto [ss, ti, dims] = mIcons.GetInventoryMiscIcon(containerType);
    mContainerTypeDisplay.SetTexture(ss, ti);
    mContainerTypeDisplay.CenterImage(dims);
}

void InventoryScreen::ShowContainer()
{
    mDisplayContainer = true;
    mSelectedCharacter.reset();
}

void InventoryScreen::ShowCharacter(BAK::ActiveCharIndex character)
{
    mDisplayContainer = false;
    mSelectedCharacter = character;
    mGameState.SetActiveCharacter(GetCharacter(character).mCharacterIndex);
}

void InventoryScreen::SetSelectionMode(bool mode, std::function<void(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>>)>&& itemSelected)
{
    SetSelectedCharacter(BAK::ActiveCharIndex{0});
    mItemSelectionMode = mode;
    mItemSelectionCallback = std::move(itemSelected);
}

void InventoryScreen::TransferItemFromCharacterToCharacter(
    InventorySlot& slot,
    unsigned amount,
    BAK::ActiveCharIndex source,
    BAK::ActiveCharIndex dest)
{
    auto item = slot.GetItem();

    if (item.IsEquipped() 
        && (item.IsItemType(BAK::ItemType::Sword)
            || item.IsItemType(BAK::ItemType::Staff)))
    {
        auto& srcC = GetCharacter(source);
        auto& dstC = GetCharacter(dest);

        if (dstC.CanSwapItem(item))
        {
            ASSERT(srcC.IsSwordsman() == dstC.IsSwordsman());
            const auto sourceItem = item;
            const auto destItemIt = dstC.GetInventory()
                .FindEquipped(item.GetObject().mType);
            const auto destItem = *destItemIt;
            ASSERT(destItemIt != dstC.GetInventory().GetItems().end());
            const auto dstIndex = std::distance(
                dstC.GetInventory().GetItems().begin(),
                destItemIt);

            dstC.GetInventory().RemoveItem(
                BAK::InventoryIndex{static_cast<unsigned>(dstIndex)});
            dstC.GiveItem(sourceItem);
            srcC.GetInventory().RemoveItem(slot.GetItemIndex());
            srcC.GiveItem(destItem);

            srcC.CheckPostConditions();
            dstC.CheckPostConditions();
            AudioA::AudioManager::Get().PlaySound(DRAG_SOUND);
        }
        else
        {
            StartDialog(BAK::DialogSources::mCantDiscardOnlyWeapon);
        }

        return;
    }

    // Reduce item amount to chosen amount
    item.SetQuantity(amount);

    if (GetCharacter(dest).CanAddItem(item))
    {
        GetCharacter(dest).GiveItem(item);
        if (item.IsStackable())
            GetCharacter(source)
                .GetInventory()
                .RemoveItem(item);
        else
            GetCharacter(source)
                .GetInventory()
                .RemoveItem(slot.GetItemIndex());

        AudioA::AudioManager::Get().PlaySound(DRAG_SOUND);
    }
    else
    {
        // Set source and dest cahracter indices here..
        mGameState.SetDialogContext(1);
        StartDialog(BAK::DialogSources::mContainerHasNoRoomForItem);
    }

    mLogger.Debug() << __FUNCTION__ << " Source: " 
        << GetCharacter(source).GetInventory() 
        << "\n" << "Dest: " << GetCharacter(dest).GetInventory() << "\n";
    GetCharacter(source).CheckPostConditions();
}

void InventoryScreen::TransferItemFromContainerToCharacter(
    InventorySlot& slot,
    BAK::ActiveCharIndex character,
    bool share,
    unsigned amount)
{
    ASSERT(mContainer);

    auto item = slot.GetItem();
    item.SetQuantity(amount);

    if (item.IsMoney() || item.IsKey())
    {
        ASSERT(mDisplayContainer);
        mGameState.GetParty().AddItem(item);
        mContainer->GetInventory()
            .RemoveItem(slot.GetItemIndex());
        AudioA::AudioManager::Get().PlaySound(DRAG_SOUND);
    }
    else if (GetCharacter(character).GiveItem(item))
    {
        if (item.IsStackable())
            mContainer->GetInventory().RemoveItem(item);
        else
            mContainer->GetInventory().RemoveItem(slot.GetItemIndex());

        AudioA::AudioManager::Get().PlaySound(DRAG_SOUND);
    }
    else
    {
    }
}

void InventoryScreen::SellItem(
    InventorySlot& slot,
    BAK::ActiveCharIndex character)
{
    ASSERT(mContainer);
    mGameState.GetParty().GainMoney(
        mShopScreen.GetBuyPrice(slot.GetItem()));
    mContainer->GiveItem(slot.GetItem());

    GetCharacter(character).GetInventory()
        .RemoveItem(slot.GetItemIndex());

    AudioA::AudioManager::Get().PlaySound(BUY_SOUND);

    mNeedRefresh = true;
}

void InventoryScreen::BuyItem(
    InventorySlot& slot,
    BAK::ActiveCharIndex character,
    bool share,
    unsigned amount)
{
    ASSERT(mContainer);

    auto item = slot.GetItem();
    item.SetQuantity(amount);
    const auto price = mShopScreen.GetSellPrice(slot.GetItemIndex(), amount);
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
            const auto result = GetCharacter(character).GiveItem(item);
            ASSERT(result);
        }
        mGameState.GetParty().LoseMoney(price);

        AudioA::AudioManager::Get().PlaySound(BUY_SOUND);
    }

    mNeedRefresh = true;
}

void InventoryScreen::TransferItemToShop(
    InventorySlot& slot,
    BAK::ActiveCharIndex character)
{
    ASSERT(mContainer);

    const auto& item = slot.GetItem();
    if (item.IsEquipped() 
        && (item.IsItemType(BAK::ItemType::Sword)
            || item.IsItemType(BAK::ItemType::Staff)))
    {
        StartDialog(BAK::DialogSources::mCantDiscardOnlyWeapon);
        return;
    }

    if (mShopScreen.CanBuyItem(slot.GetItem()))
    {
        mGameState.SetItemValue(mShopScreen.GetBuyPrice(slot.GetItem()));
        StartDialog(BAK::DialogSources::mSellItemDialog);

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
    else if (false) // Shop has no room for item, what triggers this??
    {
        mGameState.SetDialogContext(0xa);
        StartDialog(BAK::DialogSources::mContainerHasNoRoomForItem);
    }
    else
    {
        StartDialog(BAK::DialogSources::mShopWontBuyItem);
    }
}

void InventoryScreen::TransferItemFromShopToCharacter(
    InventorySlot& slot,
    BAK::ActiveCharIndex character,
    bool share,
    unsigned amount)
{
    ASSERT(mContainer);
    const auto itemIndex = slot.GetItem().GetItemIndex();
    if ((itemIndex == BAK::sBrandy || itemIndex == BAK::sAle)
        && mGameState.GetParty().GetCharacter(character)
            .GetConditions().GetCondition(BAK::Condition::Drunk).Get() >= 100)
    {
        StartDialog(BAK::DialogSources::mCantBuyTooDrunk);
        return;
    }

    if (GetCharacter(character).CanAddItem(slot.GetItem()))
    {
        const auto sellPrice = mShopScreen.GetSellPrice(slot.GetItemIndex(), amount);
        if (sellPrice == BAK::sUnpurchaseablePrice)
        {
            return;
        }

        mGameState.SetItemValue(sellPrice);
        StartDialog(BAK::DialogSources::mBuyItemDialog);

        mDialogScene.SetDialogFinished(
            [this, &slot, character, share, amount](const auto& choice)
            {
                ASSERT(choice);
                if (choice->mValue == 0x104)
                {
                    BuyItem(slot, character, share, amount);
                    mDialogScene.ResetDialogFinished();
                }
                else if (choice->mValue == 0x106)
                {
                    HaggleItem(slot, character);
                }
            });
    }
    else
    {
        mGameState.SetDialogContext(0xb);
        StartDialog(BAK::DialogSources::mContainerHasNoRoomForItem);
    }
}

void InventoryScreen::HaggleItem(
    InventorySlot& slot,
    BAK::ActiveCharIndex character)
{
    ASSERT(mContainer);

    mGameState.SetActiveCharacter(GetCharacter(character).mCharacterIndex);
    const auto item = slot.GetItem();
    if (item.GetItemIndex() == BAK::sScroll)
    {
        mDialogScene.ResetDialogFinished();
        StartDialog(BAK::DialogSources::mCantHaggleScroll);
        return;
    }

    const auto result = BAK::Haggle::TryHaggle(
        mGameState.GetParty(),
        character,
        mContainer->GetShop(),
        item.GetItemIndex(),
        mShopScreen.GetDiscount(slot.GetItemIndex()).mValue);

    if (!result)
    {
        mDialogScene.ResetDialogFinished();
        StartDialog(BAK::DialogSources::mFailHaggleItemUnavailable);
        mNeedRefresh = true;
        return;
    }

    const auto value = BAK::Royals{*result};
    mShopScreen.SetItemDiscount(slot.GetItemIndex(), value);

    if (result && value == BAK::sUnpurchaseablePrice)
    {
        mDialogScene.ResetDialogFinished();
        StartDialog(BAK::DialogSources::mFailHaggleItemUnavailable);
        mNeedRefresh = true;
    }
    else
    {
        StartDialog(BAK::DialogSources::mSucceedHaggle);
        mDialogScene.SetDialogFinished(
            [this, &slot, character](const auto& choice)
            {
                SplitStackBeforeTransferItemToCharacter(slot, character);
            });
        // So that the slot reference above is not invalid
        mNeedRefresh = false;
    }
}

void InventoryScreen::TransferItemToCharacter(
    InventorySlot& slot,
    BAK::ActiveCharIndex character,
    bool share,
    unsigned amount)
{
    CheckExclusivity();

    if (mSelectedCharacter && (*mSelectedCharacter != character))
    {
        TransferItemFromCharacterToCharacter(
            slot,
            amount,
            *mSelectedCharacter,
            character);
    }
    else
    {
        if (mContainer->IsShop())
        {
            const auto* shopItem = dynamic_cast<const ShopItemSlot*>(&slot);
            ASSERT(shopItem);
            if (shopItem->GetAvailable())
            {
                TransferItemFromShopToCharacter(
                    slot,
                    character,
                    share,
                    amount);
            }
        }
        else
        {
            TransferItemFromContainerToCharacter(
                slot,
                character,
                share,
                amount);
        }
    }

    GetCharacter(character).CheckPostConditions();
    mNeedRefresh = true;
}

void InventoryScreen::SplitStackBeforeTransferItemToCharacter(
    InventorySlot& slot,
    BAK::ActiveCharIndex character)
{
    // Can't transfer items when displaying keys
    if (mDisplayContainer && mContainer == nullptr)
        return;
    // Can't transfer items to self
    if (mSelectedCharacter && (*mSelectedCharacter == character))
        return;

    // Can't buy split stacks from shops
    if (slot.GetItem().IsStackable() 
        && (!mContainer || !mContainer->IsShop()))
    {
        const auto maxAmount = GetCharacter(character).GetInventory()
            .CanAddCharacter(slot.GetItem());

        mSplitStackDialog.BeginSplitDialog(
            [&, character](bool share, unsigned amount){
                mGuiManager.GetScreenStack().PopScreen();
                TransferItemToCharacter(slot, character, share, amount);
            },
            maxAmount);
        mGuiManager.GetScreenStack().PushScreen(&mSplitStackDialog);
    }
    else
    {
        TransferItemToCharacter(
            slot,
            character,
            false,
            slot.GetItem().GetQuantity());
    }
}

void InventoryScreen::MoveItemToEquipmentSlot(
    InventorySlot& item,
    BAK::ItemType slot)
{
    ASSERT(mSelectedCharacter);
    auto& character = GetCharacter(*mSelectedCharacter) ;

    mLogger.Debug() << "Move item to equipment slot: " 
        << item.GetItem() << " " << BAK::ToString(slot) << "\n";

    const auto& slotItem = item.GetItem();
    const auto weaponSlotType = character.IsSwordsman()
        ? BAK::ItemType::Sword
        : BAK::ItemType::Staff;

    if (slot == BAK::ItemType::Sword
        && (slotItem.IsItemType(BAK::ItemType::Sword)
            || slotItem.IsItemType(BAK::ItemType::Staff)))
    {
        character.ApplyItemToSlot(item.GetItemIndex(), weaponSlotType);
    }
    else if (slot == BAK::ItemType::Crossbow && slotItem.IsItemType(BAK::ItemType::Crossbow))
    {
        character.ApplyItemToSlot(item.GetItemIndex(), slot);
    }
    else if (slot == BAK::ItemType::Armor && slotItem.IsItemType(BAK::ItemType::Armor))
    {
        character.ApplyItemToSlot(item.GetItemIndex(), slot);
    }
    else
    {
        UseItem(item, character.GetItemAtSlot(slot));
    }

    GetCharacter(*mSelectedCharacter).CheckPostConditions();

    mNeedRefresh = true;
}

void InventoryScreen::MoveItemToContainer(
    InventorySlot& slot,
    bool share,
    unsigned amount)
{
    // Can't move an item in a container to the container...
    ASSERT(!mDisplayContainer);
    ASSERT(mSelectedCharacter);

    auto item = slot.GetItem();
    item.SetQuantity(amount);

    if (item.IsEquipped() 
        && (item.IsItemType(BAK::ItemType::Sword)
            || item.IsItemType(BAK::ItemType::Staff)))
    {
        StartDialog(BAK::DialogSources::mCantDiscardOnlyWeapon);
        return;
    }
    mLogger.Debug() << "Move item to container: " << item << "\n";

    if (mContainer && mContainer->IsShop())
    {
        ASSERT(mSelectedCharacter);
        TransferItemToShop(
            slot,
            *mSelectedCharacter);
    }
    else if (mContainer && mContainer->CanAddItem(item))
    {
        mContainer->GetInventory().AddItem(item);
        if (item.IsStackable())
            GetCharacter(*mSelectedCharacter).GetInventory()
                .RemoveItem(item);
        else
            GetCharacter(*mSelectedCharacter).GetInventory()
                .RemoveItem(slot.GetItemIndex());
    }
    else
    {
        StartDialog(BAK::DialogSources::mContainerHasNoRoomForItem);
    }

    GetCharacter(*mSelectedCharacter).CheckPostConditions();
    mNeedRefresh = true;
}

void InventoryScreen::SplitStackBeforeMoveItemToContainer(InventorySlot& slot)
{
    if (mDisplayContainer || !mContainer)
        return;

    // Game doesn't split stacks when selling to shops
    if (slot.GetItem().IsStackable() && !mContainer->IsShop())
    {
        const auto maxAmount = mContainer->GetInventory()
            .CanAddContainer(slot.GetItem());

        mSplitStackDialog.BeginSplitDialog(
            [&](bool share, unsigned amount){
                mGuiManager.GetScreenStack().PopScreen();
                MoveItemToContainer(slot, share, amount);
            },
            maxAmount);
        mGuiManager.GetScreenStack().PushScreen(&mSplitStackDialog);
    }
    else
    {
        MoveItemToContainer(slot, false, slot.GetItem().GetQuantity());
    }
}

void InventoryScreen::UseItem(InventorySlot& sourceItemSlot, BAK::InventoryIndex targetItemIndex)
{
    ASSERT(mSelectedCharacter);
    auto& character = GetCharacter(*mSelectedCharacter) ;
    [[maybe_unused]] const auto result = BAK::ApplyItemTo(character, sourceItemSlot.GetItemIndex(), targetItemIndex);

    character.CheckPostConditions();

    mNeedRefresh = true;
}

void InventoryScreen::AdvanceNextPage()
{
    mLogger.Debug() << __FUNCTION__ << "\n";
    if (mDisplayContainer)
    {
        mShopScreen.AdvanceNextPage();
        mNeedRefresh = true;
    }
}

void InventoryScreen::ShowItemDescription(const BAK::InventoryItem& item)
{
    mDetails.AddItem(item, mGameState);
    mDisplayDetails = true;
    mNeedRefresh = true;
}

void InventoryScreen::HighlightValidDrops(const InventorySlot& slot)
{
    const auto& party = mGameState.GetParty();
    BAK::ActiveCharIndex person{0};
    do
    {
        if (person != mSelectedCharacter)
        {
            const auto& item = slot.GetItem();
            const auto mustSwap = item.IsEquipped() 
                && (item.IsItemType(BAK::ItemType::Sword)
                    || item.IsItemType(BAK::ItemType::Staff));
            const auto giveable = GetCharacter(person)
                .CanAddItem(slot.GetItem());
            const auto* shopItem = dynamic_cast<const ShopItemSlot*>(&slot);
            if (mDisplayContainer && mContainer && mContainer->IsShop())
            {
                ASSERT(shopItem);
            }
            const auto isAvailableShopItem = (!mDisplayContainer || !mContainer->IsShop()) || (mContainer->IsShop()
                && mDisplayContainer
                && shopItem->GetAvailable());
            if (isAvailableShopItem 
                && ((mustSwap && (GetCharacter(person).CanSwapItem(item)))
                || (giveable && !mustSwap)))
            {
                mCharacters[person.mValue].SetColor(glm::vec4{.0, .05, .0, 1}); 
                mCharacters[person.mValue].SetColorMode(Graphics::ColorMode::TintColor);
            }
            else
            {
                mCharacters[person.mValue].SetColor(glm::vec4{.05, .0, .0, 1}); 
                mCharacters[person.mValue].SetColorMode(Graphics::ColorMode::TintColor);
            }
        }

        person = party.NextActiveCharacter(person);
    } while (person != BAK::ActiveCharIndex{0});
}

void InventoryScreen::UnhighlightDrops()
{
    const auto& party = mGameState.GetParty();
    BAK::ActiveCharIndex person{0};
    do
    {
        if (person != mSelectedCharacter)
        {
                mCharacters[person.mValue].SetColor(glm::vec4{.05, .05, .05, 1}); 
                mCharacters[person.mValue].SetColorMode(Graphics::ColorMode::TintColor);
        }

        person = party.NextActiveCharacter(person);
    } while (person != BAK::ActiveCharIndex{0});
}

void InventoryScreen::UpdatePartyMembers()
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
                SplitStackBeforeTransferItemToCharacter(slot, character);
            },
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

void InventoryScreen::UpdateGold()
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

void InventoryScreen::UpdateInventoryContents()
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
        const auto& [ss, ti, _] = mIcons.GetInventoryIcon(item.GetItemIndex().mValue);

        if ((item.IsItemType(BAK::ItemType::Sword)
            || item.IsItemType(BAK::ItemType::Staff))
            && item.IsEquipped())
        {
            auto scale = slotDims * glm::vec2{2, 1};
            if (item.IsItemType(BAK::ItemType::Staff))
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

        if (item.IsItemType(BAK::ItemType::Crossbow)
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

        if (item.IsItemType(BAK::ItemType::Armor)
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

void InventoryScreen::AddChildren()
{
    AddChildBack(&mFrame);

    if (mDisplayDetails)
    {
        AddChildBack(&mDetails);
    }

    AddChildBack(&mExit);
    AddChildBack(&mGoldDisplay);

    AddChildBack(&mContainerTypeDisplay);

    for (auto& character : mCharacters)
    {
        AddChildBack(&character);
    }

    if (mDisplayDetails)
        return;

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

void InventoryScreen::CheckExclusivity()
{
    ASSERT(bool{mSelectedCharacter} ^ mDisplayContainer);
}

void InventoryScreen::HandleItemSelected()
{
    const auto checkItem = [&](auto& item) -> bool
    {
        if (item.IsSelected())
        {
            mSelectedItem = item.GetItemIndex();
            ASSERT(mItemSelectionCallback);
            mItemSelectionCallback(GetSelectedItem());
            item.ResetSelected();
            return true;
        }
        return false;
    };

    for (auto& item : mInventoryItems)
    {
        if (checkItem(item))
            return;
    }

    if (mWeapon.HasItem() && checkItem(mWeapon.GetInventorySlot()))
        return;

    if (mArmor.HasItem() && checkItem(mArmor.GetInventorySlot()))
        return;

    if (mCrossbow.HasItem() && checkItem(mCrossbow.GetInventorySlot()))
        return;
}

std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>> InventoryScreen::GetSelectedItem() const
{
    if (mSelectedItem && mSelectedCharacter)
    {
        return std::make_pair(*mSelectedCharacter, *mSelectedItem);
    }

    return std::nullopt;
}

}
