#pragma once

#include "bak/dialog.hpp"
#include "bak/condition.hpp"
#include "bak/money.hpp"
#include "bak/shop.hpp"
#include "bak/types.hpp"

namespace BAK::Temple {

static constexpr auto sTempleOfSung = 4;
static constexpr auto sChapelOfIshap = 12;

bool CanBlessItem(const BAK::InventoryItem& item);
bool IsBlessed(const BAK::InventoryItem& item);
Royals CalculateBlessPrice(const BAK::InventoryItem& item, const ShopStats& shop);
void BlessItem(BAK::InventoryItem& item, const ShopStats& shop);
void RemoveBlessing(BAK::InventoryItem& item);

Royals CalculateTeleportCost(unsigned source, unsigned dest);

Royals CalculateCureCost(unsigned cureFactor, bool isTempleOfSung, Skills&, const Conditions&);
void CureCharacter(Skills&, Conditions&, bool isTempleOfSung);

}
