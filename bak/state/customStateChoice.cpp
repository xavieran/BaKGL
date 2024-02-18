#include "bak/state/customStateChoice.hpp"

#include "bak/gameState.hpp"

namespace BAK::State {

CustomStateEvaluator::CustomStateEvaluator(const GameState& gameState)
:
    mGameState{gameState}
{}

bool CustomStateEvaluator::AnyCharacterStarving() const
{
    bool foundStarving = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            if (character.GetConditions().GetCondition(Condition::Starving).Get() > 0)
            {
                foundStarving = true;
                return Loop::Finish;
            }
            return Loop::Continue;
        });

    return foundStarving;
}

bool CustomStateEvaluator::Plagued() const
{
    bool foundPlagued = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            if (character.GetConditions().GetCondition(Condition::Plagued).Get() > 0)
            {
                foundPlagued = true;
                return Loop::Finish;
            }
            return Loop::Continue;
        });

    return foundPlagued;
}

bool CustomStateEvaluator::HaveSixSuitsOfArmor() const
{
    unsigned armorCount = 0;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            const auto& items = character.GetInventory().GetItems();
            for (const auto& item : items)
            {
                if (item.GetItemIndex() == sStandardArmor)
                {
                    armorCount++;
                }
            }
            return Loop::Continue;
        });
    return armorCount >= 6;
}

bool CustomStateEvaluator::AllPartyArmorIsGoodCondition() const
{
    bool foundRepairableArmor = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            const auto& items = character.GetInventory().GetItems();
            for (const auto& item : items)
            {
                if (item.IsItemType(ItemType::Armor) && item.IsRepairableByShop())
                {
                    foundRepairableArmor = true;
                    return Loop::Finish;
                }
            }
            return Loop::Continue;
        });

    return !foundRepairableArmor;
}

bool CustomStateEvaluator::PoisonedDelekhanArmyChests() const
{
    // Check for poisoned rations in
    // 5, 0x16b2fb, 0x111547
    // 5, 0x16b2fb, 0x110f20
    // 5, 0x16b33a, 0x11083c
    return false;
}

bool CustomStateEvaluator::AnyCharacterSansWeapon() const
{
    bool noWeapon = false;
    mGameState.GetParty().ForEachActiveCharacter(
        [&](const auto& character){
            if (character.HasEmptyStaffSlot() || character.HasEmptySwordSlot())
            {
                noWeapon = true;
                return Loop::Finish;
            }
            return Loop::Continue;
        });

    return !noWeapon;
}

bool CustomStateEvaluator::AnyCharacterHasNegativeCondition() const
{
    bool nonZero = false;
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        for (unsigned i = 0; i < 7; i++)
        {
            if (i == 4) continue;
            if (character.GetConditions().GetCondition(static_cast<Condition>(i)).Get() > 0)
            {
                nonZero = true;
                return Loop::Finish;
            }
        }
        return Loop::Continue;
    });
    return nonZero;
}

bool CustomStateEvaluator::AnyCharacterIsUnhealthy() const
{
    if (AnyCharacterHasNegativeCondition())
    {
        return true;
    }

    bool nonZero = false;
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        if (character.GetSkill(SkillType::TotalHealth) != character.GetMaxSkill(SkillType::TotalHealth))
        {
            nonZero = true;
            return Loop::Finish;
        }
        return Loop::Continue;
    });
    return nonZero;
}

bool CustomStateEvaluator::AllCharactersHaveNapthaMask() const
{
    bool noMask = false;
    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        if (!character.GetInventory().HaveItem(
            InventoryItemFactory::MakeItem(sNapthaMask, 1)))
        {
            noMask = true;
            return Loop::Finish;
        }
        return Loop::Continue;
    });
    return noMask;
}

bool CustomStateEvaluator::NormalFoodInArlieChest() const
{
    const auto zone = 3;
    const auto x = 1308000;
    const auto y = 1002400;
    // GetContainer(zone, x, y).HasItem(sRations);
    return false;
}

bool CustomStateEvaluator::PoisonedFoodInArlieChest() const
{
    const auto zone = 3;
    const auto x = 1308000;
    const auto y = 1002400;
    // GetContainer(zone, x, y).HasItem(sPoisonedRations);
    return false;
}

}
