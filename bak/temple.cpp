#include "bak/temple.hpp"
#include "bak/skills.hpp"

namespace BAK::Temple {

bool CanBlessItem(const BAK::InventoryItem& item)
{
    const auto& object = item.GetObject();
    return object.mType == ItemType::Armor
        || object.mType == ItemType::Sword;
}

bool IsBlessed(const BAK::InventoryItem& item)
{
    return item.HasModifier(Modifier::Blessing1)
        || item.HasModifier(Modifier::Blessing2)
        || item.HasModifier(Modifier::Blessing3);
}

Royals CalculateBlessPrice(const BAK::InventoryItem& item, const ShopStats& shop)
{
    const auto& object = item.GetObject();
    const auto cost = (shop.GetTempleBlessFixedCost() * 10) + ((object.mValue * shop.GetTempleBlessPercent()) / 100);
    assert(cost > 0);
    return Royals{static_cast<unsigned>(cost)};
}

void BlessItem(BAK::InventoryItem& item, const ShopStats& shop)
{
    item.SetModifier(shop.GetTempleBlessType());
}

void RemoveBlessing(BAK::InventoryItem& item)
{
    for (const auto blessType : {Modifier::Blessing1, Modifier::Blessing2, Modifier::Blessing3})
    {
        item.UnsetModifier(blessType);
    }
}

Royals CalculateTeleportCost(unsigned source, unsigned dest)
{
    ASSERT(source > 0 && dest > 0);
    ASSERT(source <= 13 && dest <= 13);
    static constexpr unsigned costMatrix[12][12] = {
        {0, 85, 88, 112, 167, 149, 172, 184, 106, 159, 120, 158},
        {85, 0, 70, 105, 136, 123, 138, 139,  61, 113, 119, 129},
        {88, 70, 0,  84, 129, 111, 140, 152,  83, 127,  99, 120},
        {88, 70, 105, 0,  99,  81, 134, 146, 103, 134,  59,  91},
        {88, 70, 105, 99,  0,  70, 102, 131, 129, 124, 105,  61},
        {88, 70, 105, 99, 70,   0, 102, 131, 115, 121,  75,  49},
        {88, 70, 105, 99, 70, 102,   0,  89, 137,  97, 151, 120},
        {88, 70, 105, 99, 70, 102,  89,   0, 128,  75, 161, 140},
        {88, 70, 105, 99, 70, 102,  89, 128,   0, 102, 121, 131},
        {88, 70, 105, 99, 70, 102,  89, 128, 102,   0, 151, 129},
        {88, 70, 105, 99, 70, 102,  89, 128, 102, 151,   0,  63},
        {88, 70, 105, 99, 70, 102,  89, 128, 102, 151,  63,   0}
    };
    return GetRoyals(Sovereigns{costMatrix[source - 1][dest - 1]});
}

Royals CalculateCureCost(unsigned cureFactor, bool isTempleOfSung, Skills& skills, const Conditions& conditions)
{
    static constexpr unsigned conditionCost[7] = {
        4, 10, 10, 3, 0, 2, 30
    };

    unsigned totalCost = 0;
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = conditions.GetCondition(static_cast<Condition>(i));
        if (i != static_cast<unsigned>(Condition::Healing))
            const auto cost = cond.Get() * conditionCost[i] + 10;
    }

    totalCost = (totalCost * cureFactor) / 100;

    if (isTempleOfSung)
    {
        totalCost += CalculateEffectiveSkillValue(
            SkillType::TotalHealth,
            skills,
            conditions,
            SkillRead::MaxSkill);
        totalCost -= CalculateEffectiveSkillValue(
            SkillType::TotalHealth,
            skills,
            conditions,
            SkillRead::Current);
    }

    return Royals{totalCost};
}

void CureCharacter(Skills& skills, Conditions& conditions, bool isTempleOfSung)
{
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = static_cast<Condition>(i);
        const auto amount = cond == Condition::Healing ? 20 : -100;
        conditions.AdjustCondition(cond, amount);
    }
    
    if (isTempleOfSung)
    {
        DoAdjustHealth(skills, 100, 0x7fff);
        conditions.AdjustCondition(Condition::Healing, 100);
    }
}

}
