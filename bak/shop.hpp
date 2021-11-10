#pragma once

#include "bak/IContainer.hpp"
#include "bak/money.hpp"
#include "bak/inventoryItem.hpp"

namespace BAK::Shop {

Royals GetSellPrice(const BAK::InventoryItem&, const ShopStats&);
Royals GetBuyPrice (const BAK::InventoryItem&, const ShopStats&);

double GetItemQuantityMultiple(const BAK::InventoryItem&);

}
