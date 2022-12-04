#pragma once

#include "bak/character.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/objectInfo.hpp"


namespace BAK {

struct ItemUseResult
{

};

ItemUseResult ApplyItemTo(Character&, InventoryIndex sourceItem, InventoryIndex targetItem);

ItemUseResult RepairItem(Character&, InventoryIndex sourceItem, InventoryIndex targetItem);

}
