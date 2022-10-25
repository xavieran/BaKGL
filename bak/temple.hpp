#pragma once

#include "bak/dialog.hpp"
#include "bak/money.hpp"
#include "bak/shop.hpp"
#include "bak/types.hpp"

namespace BAK::Temple {

bool CanBlessItem(const BAK::InventoryItem& item);
bool IsBlessed(const BAK::InventoryItem& item);
Royals CalculateBlessPrice(const BAK::InventoryItem& item, const ShopStats& shop);
void BlessItem(BAK::InventoryItem& item, const ShopStats& shop);
void RemoveBlessing(BAK::InventoryItem& item);

}
