#pragma once

#include "bak/IContainer.hpp"

#include "bak/condition.hpp"
#include "bak/inventory.hpp"
#include "bak/skills.hpp"
#include "bak/spells.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

#include <cstdint>
#include <ostream>
#include <string>

namespace BAK {

class Character final : public IContainer
{
public:
    Character(
        unsigned index,
        const std::string& name,
        const Skills& skills,
        Spells spells,
        const std::array<std::uint8_t, 2>& unknown,
        const std::array<std::uint8_t, 7>& unknown2,
        const Conditions& conditions,
        Inventory&& inventory);

    /* IContainer */
    Inventory& GetInventory() override;
    const Inventory& GetInventory() const override;
    bool CanAddItem(const InventoryItem& ref) const override;
    bool GiveItem(const InventoryItem& ref) override;
    bool RemoveItem(const InventoryItem& item) override;
    ContainerType GetContainerType() const override;
    ShopStats& GetShop() override;
    const ShopStats& GetShop() const override;
    LockStats& GetLock() override;

    /* Character Getters */
    bool CanSwapItem(const InventoryItem& ref) const;

    CharIndex GetIndex() const;

    bool IsSpellcaster() const;
    bool IsSwordsman() const;

    bool HasEmptyStaffSlot() const;
    bool HasEmptySwordSlot() const;
    bool HasEmptyCrossbowSlot() const;
    bool HasEmptyArmorSlot() const;
    InventoryIndex GetItemAtSlot(ItemType slot) const;
    ItemType GetWeaponType() const;
    bool CanReplaceEquippableItem(ItemType type) const;
    void ApplyItemToSlot(InventoryIndex index, ItemType slot);
    void CheckPostConditions();
    unsigned GetTotalItem(const std::vector<BAK::ItemIndex>& itemIndex);
    const std::string& GetName() const;

    bool CanHeal(bool isInn);
    bool HaveNegativeCondition();

    const Skills& GetSkills() const;
    Skills& GetSkills();

    void ImproveSkill(SkillType skill, SkillChange skillChangeType, int multiplier);
    unsigned GetSkill(SkillType skill) const;
    unsigned GetMaxSkill(SkillType skill) const;
    void AdjustCondition(BAK::Condition cond, signed amount);
    const Conditions& GetConditions() const;
    Conditions& GetConditions();

    void UpdateSkills();

    Spells& GetSpells();
    const Spells& GetSpells() const;

    void AddSkillAffector(const SkillAffector& affector);
    std::vector<SkillAffector>& GetSkillAffectors();
    const std::vector<SkillAffector>& GetSkillAffectors() const;

    CharIndex mCharacterIndex;
    std::string mName;
    mutable Skills mSkills;
    Spells mSpells;
    std::array<std::uint8_t, 2> mUnknown;
    std::array<std::uint8_t, 7> mUnknown2;
    Conditions mConditions;
    Inventory mInventory;
    std::vector<SkillAffector> mSkillAffectors;

    const Logging::Logger& mLogger;
};

std::ostream& operator<<(std::ostream&, const Character&);

}
