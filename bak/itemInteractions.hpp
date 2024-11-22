#pragma once

#include "bak/dialogTarget.hpp"
#include "bak/types.hpp"

#include <optional>

namespace BAK {

class Character;
class GameState;

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
