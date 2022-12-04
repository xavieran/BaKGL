#include "bak/itemInteractions.hpp"

#include "bak/skills.hpp"

#include "com/logger.hpp"

namespace BAK {

ItemUseResult ApplyItemTo(
    Character& character,
    InventoryIndex sourceItemIndex,
    InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);
    if (sourceItem.IsRepairItem() 
        && targetItem.IsItemType(static_cast<ItemType>(sourceItem.GetObject().mEffectMask))
        && targetItem.IsRepairable())
    {
        Logging::LogDebug(__FUNCTION__) << " Repair " << targetItem << " with " << sourceItem << "\n";
        RepairItem(character, sourceItemIndex, targetItemIndex);
    }
    else if (sourceItem.IsItemModifier()
        && targetItem.IsModifiableBy(sourceItem.GetObject().mType))
    {
        Logging::LogDebug(__FUNCTION__) << " Apply Modifier " << targetItem << " with " << sourceItem << "\n";
        character.RemoveItem(sourceItemIndex, 1);
        targetItem.SetModifier(static_cast<Modifier>(sourceItem.GetObject().mEffectMask));
    }
    else
    {
        Logging::LogDebug(__FUNCTION__) << " DO NOTHING " << targetItem << " with " << sourceItem << "\n";
    }

    return ItemUseResult{};
}

ItemUseResult RepairItem(Character& character, InventoryIndex sourceItemIndex, InventoryIndex targetItemIndex)
{
    auto& sourceItem = character.GetInventory().GetAtIndex(sourceItemIndex);
    auto& targetItem = character.GetInventory().GetAtIndex(targetItemIndex);

    const auto whichSkill = targetItem.IsItemType(ItemType::Armor)
        ? SkillType::Armorcraft
        : SkillType::Weaponcraft;
    const auto skill = character.GetSkill(whichSkill);
    character.ImproveSkill(whichSkill, SkillChange::ExercisedSkill, 1);

    const auto initialCondition = targetItem.GetCondition();
    const auto adjustment = ((100 - initialCondition) * skill) / 100;
    auto finalCondition = initialCondition + adjustment;
    if (finalCondition > 99)
        finalCondition = 99;

    targetItem.SetCondition(finalCondition);
    targetItem.SetRepairable(false);
    character.RemoveItem(sourceItemIndex, 1);
    return ItemUseResult{};
}

}
