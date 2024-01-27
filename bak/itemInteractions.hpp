#pragma once

#include "bak/character.hpp"
#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"


namespace BAK {

struct ItemUseResult
{
    std::optional<std::pair<unsigned, unsigned>> mUseSound;
    std::optional<unsigned> mDialogContext;
    Target mDialog;
};

ItemUseResult ApplyItemTo(
    Character&,
    InventoryIndex sourceItem,
    InventoryIndex targetItem);

ItemUseResult UseItem(
    GameState& gameState,
    Character&,
    InventoryIndex sourceItem);

}
