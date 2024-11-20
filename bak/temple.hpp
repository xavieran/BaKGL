#pragma once

#include "bak/types.hpp"

#include <glm/fwd.hpp>
#include <vector>

namespace BAK {
class Conditions;
class InventoryItem;
class Skills;
struct SkillAffector;
struct ShopStats;
}

namespace BAK::Temple {

static constexpr auto sTempleOfSung = 4;
static constexpr auto sChapelOfIshap = 12;

bool CanBlessItem(const InventoryItem& item);
bool IsBlessed(const InventoryItem& item);
Royals CalculateBlessPrice(const InventoryItem& item, const ShopStats& shop);
void BlessItem(InventoryItem& item, const ShopStats& shop);
void RemoveBlessing(InventoryItem& item);

Royals CalculateTeleportCost(
        unsigned srcTemple,
        unsigned dstTemple,
        glm::vec2 srcPos,
        glm::vec2 dstPos,
        unsigned teleportMultiplier,
        unsigned teleportConstant);

Royals CalculateCureCost(
    unsigned cureFactor,
    bool isTempleOfSung,
    Skills&,
    const Conditions&,
    const std::vector<SkillAffector>&);
void CureCharacter(Skills&, Conditions&, bool isTempleOfSung);

}
