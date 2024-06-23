#include "bak/character.hpp"

#include "com/ostream.hpp"

#include <ios>

namespace BAK {

Character::Character(
    unsigned index,
    const std::string& name,
    const Skills& skills,
    Spells spells,
    const std::array<std::uint8_t, 2>& unknown,
    const std::array<std::uint8_t, 7>& unknown2,
    const Conditions& conditions,
    Inventory&& inventory)
:
    mCharacterIndex{index},
    mName{name},
    mSkills{skills},
    mSpells{spells},
    mUnknown{unknown},
    mUnknown2{unknown2},
    mConditions{conditions},
    mInventory{std::move(inventory)},
    mSkillAffectors{},
    mLogger{Logging::LogState::GetLogger("BAK::Character")}
{}

/* IContainer */

Inventory& Character::GetInventory() { return mInventory; }
const Inventory& Character::GetInventory() const { return mInventory; }

bool Character::CanSwapItem(const InventoryItem& ref) const
{
    return (ref.IsItemType(BAK::ItemType::Sword) && IsSwordsman())
        || (ref.IsItemType(BAK::ItemType::Staff) && IsSpellcaster());
}

bool Character::CanAddItem(const InventoryItem& ref) const
{
    auto item = ref;
    const auto itemIndex = ref.GetItemIndex();
    // Day's rations are equivalent to 1 of normal rations
    if (itemIndex == BAK::sDayRations)
    {
        item = InventoryItemFactory::MakeItem(BAK::sRations, 1);
    }

    if (mInventory.CanAddCharacter(item) > 0)
        return true;
    else if (item.IsItemType(ItemType::Staff)
        && HasEmptyStaffSlot())
        return true;
    else if (item.IsItemType(ItemType::Sword)
        && HasEmptySwordSlot())
        return true;
    else if (item.IsItemType(ItemType::Crossbow)
        && HasEmptyCrossbowSlot())
        return true;
    else if (item.IsItemType(ItemType::Armor)
        && HasEmptyArmorSlot())
        return true;
    else
        return false;
}

bool Character::GiveItem(const InventoryItem& ref)
{
    auto item = ref;
    const auto itemIndex = ref.GetItemIndex();

    bool equipped = false;
    if (CanReplaceEquippableItem(item.GetObject().mType)
        && mInventory.FindEquipped(item.GetObject().mType)
            == mInventory.GetItems().end())
    {
        item.SetEquipped(true);
        equipped = true;
    }
    else
    {
        item.SetEquipped(false);
    }

    // Day's rations are equivalent to 1 of normal rations
    if (itemIndex == BAK::sDayRations)
    {
        item = InventoryItemFactory::MakeItem(BAK::sRations, 1);
    }
    // Quegian brandy
    else if (itemIndex == BAK::sBrandy
        || itemIndex == BAK::sAle
        || itemIndex == BAK::sKeshianAle)
    {
        mConditions.IncreaseCondition(
            static_cast<Condition>(ref.GetObject().mEffectMask),
            ref.GetObject().mEffect);
        return true;
    }

    if (item.GetItemIndex() == sRations
        && GetConditions().GetCondition(Condition::Starving).Get() > 0)
    {
        AdjustCondition(Condition::Starving, -100);
        item.SetQuantity(item.GetQuantity() - 1);
        if (item.GetQuantity() == 0)
        {
            return true;
        }
    }
    const auto stackSize = item.GetObject().mStackSize;

    // Split into stacks if necessary
    std::vector<InventoryItem> items{};
    if (item.IsStackable()
        && item.GetQuantity() > stackSize)
    {
        const auto nStacks = item.GetQuantity() / stackSize;
        for (unsigned i = 0; i < nStacks; i++)
        {
            items.emplace_back(
                InventoryItemFactory::MakeItem(
                    ItemIndex{itemIndex},
                    stackSize));
        }
        const auto remainder = item.GetQuantity() % stackSize;
        if (remainder != 0)
            items.emplace_back(
                InventoryItemFactory::MakeItem(
                    ItemIndex{itemIndex},
                    remainder));
    }
    else
    {
        items.emplace_back(item);
    }

    bool added = false;
    for (const auto& item : items)
    {
        if (mInventory.CanAddCharacter(item) || equipped)
        {
            mInventory.AddItem(item);
            added = true;
        }
    }

    return added;
}

bool Character::RemoveItem(const InventoryItem& item)
{
    if (mInventory.HaveItem(item))
    {
        mInventory.RemoveItem(item);
        return true;
    }

    return false;
}

ContainerType Character::GetContainerType() const
{
    return ContainerType::Inv;
}

ShopStats& Character::GetShop() { ASSERT(false); return *reinterpret_cast<ShopStats*>(this);}
const ShopStats& Character::GetShop() const { ASSERT(false); return *reinterpret_cast<const ShopStats*>(this);}
LockStats& Character::GetLock() { ASSERT(false); return *reinterpret_cast<LockStats*>(this); }

/* Character Getters */

CharIndex Character::GetIndex() const { return mCharacterIndex; }

bool Character::IsSpellcaster() const { return mSkills.GetSkill(BAK::SkillType::Casting).mMax != 0; }
bool Character::IsSwordsman() const { return !IsSpellcaster(); }

bool Character::HasEmptyStaffSlot() const
{
    return IsSpellcaster() 
        && mInventory.FindEquipped(ItemType::Staff) 
            == mInventory.GetItems().end();
}

bool Character::HasEmptySwordSlot() const
{
    return IsSwordsman() 
        && mInventory.FindEquipped(ItemType::Sword) 
            == mInventory.GetItems().end();
}

bool Character::HasEmptyCrossbowSlot() const
{
    return IsSwordsman() 
        && mInventory.FindEquipped(ItemType::Crossbow) 
            == mInventory.GetItems().end();
}

bool Character::HasEmptyArmorSlot() const
{
    return mInventory.FindEquipped(ItemType::Armor) 
        == mInventory.GetItems().end();
}

InventoryIndex Character::GetItemAtSlot(ItemType slot) const
{
    auto it = mInventory.FindEquipped(slot) ;
    const auto index = mInventory.GetIndexFromIt(it);
    assert(index);
    return *index;
}

ItemType Character::GetWeaponType() const
{
    if (IsSpellcaster())
        return ItemType::Staff;
    else
        return ItemType::Sword;
}

bool Character::CanReplaceEquippableItem(ItemType type) const
{
    if (IsSpellcaster() && type == ItemType::Staff)
        return true;
    else if (IsSwordsman() && (type == ItemType::Sword
            || type == ItemType::Crossbow))
        return true;
    else if (type == ItemType::Armor)
        return true;
    return false;
}

void Character::ApplyItemToSlot(InventoryIndex index, ItemType slot)
{
    auto& item = mInventory.GetAtIndex(index);
    auto equipped = mInventory.FindEquipped(slot);

    if (equipped == mInventory.GetItems().end())
    {
        item.SetEquipped(true);
        return;
    }

    const auto slotIndex = *mInventory.GetIndexFromIt(equipped);
    // We are trying to move this item onto itself
    if (slotIndex == index)
    {
        return;
    }

    if (item.IsItemType(slot))
    {
        item.SetEquipped(true);
        if (equipped != mInventory.GetItems().end())
            equipped->SetEquipped(false);
    }
    else
    {
        // Try use item at index on slot item
        //UseItem(index, slotIndex);
    }

    Logging::LogDebug("CharacterAFMove") << __FUNCTION__ << " " << item << " " << BAK::ToString(slot) << "\n";
    if (equipped != mInventory.GetItems().end())
        Logging::LogDebug("CharacterAFEquip") << __FUNCTION__ << " " << *equipped << " " << equipped->IsEquipped() << " " << BAK::ToString(slot) << "\n";
}

void Character::CheckPostConditions()
{
    if (IsSpellcaster())
    {
        ASSERT(GetInventory().FindEquipped(ItemType::Sword) 
            == GetInventory().GetItems().end());
        ASSERT(GetInventory().FindEquipped(ItemType::Crossbow) 
            == GetInventory().GetItems().end());

        unsigned staffCount = 0;
        for (const auto& item : GetInventory().GetItems())
        {
            if (item.IsItemType(ItemType::Staff) && item.IsEquipped())
            {
                staffCount++;
            }
        }
        ASSERT(staffCount <= 1);
    }
    else
    {
        ASSERT(GetInventory().FindEquipped(ItemType::Staff) 
            == GetInventory().GetItems().end());

        unsigned swordCount = 0;
        for (const auto& item : GetInventory().GetItems())
        {
            if (item.IsItemType(ItemType::Sword) && item.IsEquipped())
            {
                swordCount++;
            }
        }
        ASSERT(swordCount <= 1);

        unsigned crossbowCount = 0;
        for (const auto& item : GetInventory().GetItems())
        {
            if (item.IsItemType(ItemType::Crossbow) && item.IsEquipped())
            {
                crossbowCount++;
            }
        }

        ASSERT(crossbowCount <= 1);
    }
}

unsigned Character::GetTotalItem(const std::vector<BAK::ItemIndex>& itemIndex)
{
    unsigned total = 0;
    for (const auto& item : GetInventory().GetItems())
    {
        if (std::find_if(itemIndex.begin(), itemIndex.end(), [&item](auto& elem){ return item.GetItemIndex() == elem;}) != itemIndex.end())
        {
            if (item.IsConditionBased() || item.IsChargeBased())
            {
                total += 1;
            }
            else
            {
                total += item.GetQuantity();
            }
        }
    }
    return total;
}

const std::string& Character::GetName() const
{
    return mName;
}

bool Character::CanHeal(bool isInn)
{
    const auto multiplier = isInn ? 1.0 : .80;
    const auto health = GetSkill(BAK::SkillType::TotalHealth);
    const auto maxHealth = GetMaxSkill(BAK::SkillType::TotalHealth);
    return health < (maxHealth * multiplier);
}

bool Character::HaveNegativeCondition()
{
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        auto cond = static_cast<BAK::Condition>(i);
        if (cond == BAK::Condition::Healing) continue;
        if (mConditions.GetCondition(cond).Get() > 0) return true;
    }
    return false;
}

const Skills& Character::GetSkills() const
{
    return mSkills;
}

Skills& Character::GetSkills()
{
    return mSkills;
}

void Character::ImproveSkill(SkillType skill, SkillChange skillChangeType, int multiplier)
{
    mSkills.ImproveSkill(mConditions, skill, skillChangeType, multiplier);
    UpdateSkills();
}

unsigned Character::GetSkill(SkillType skill) const
{
    if (skill != SkillType::TotalHealth)
    {
        mSkills.GetSkill(skill).mModifier = mInventory.CalculateModifiers(skill);
    }
    return CalculateEffectiveSkillValue(
        skill,
        mSkills,
        mConditions,
        mSkillAffectors,
        SkillRead::Current);
}

unsigned Character::GetMaxSkill(SkillType skill) const
{
    if (skill != SkillType::TotalHealth)
    {
        mSkills.GetSkill(skill).mModifier = mInventory.CalculateModifiers(skill);
    }
    return CalculateEffectiveSkillValue(
        skill,
        mSkills,
        mConditions,
        mSkillAffectors,
        SkillRead::MaxSkill);
}

void Character::AdjustCondition(BAK::Condition cond, signed amount)
{
    mConditions.AdjustCondition(mSkills, cond, amount);
}

const Conditions& Character::GetConditions() const { return mConditions; }
Conditions& Character::GetConditions() { return mConditions; }

void Character::UpdateSkills()
{
    for (unsigned i = 0; i < BAK::Skills::sSkills; i++)
        GetSkill(static_cast<SkillType>(i));
}

Spells& Character::GetSpells()
{
    return mSpells;
}

const Spells& Character::GetSpells() const
{
    return mSpells;
}

void Character::AddSkillAffector(const SkillAffector& affector)
{
    mSkillAffectors.emplace_back(affector);
}

std::vector<SkillAffector>& Character::GetSkillAffectors()
{
    return mSkillAffectors;
}

const std::vector<SkillAffector>& Character::GetSkillAffectors() const
{
    return mSkillAffectors;
}

std::ostream& operator<<(std::ostream& os, const Character& c)
{
os << "Character [" << c.mName << " Skills: \n" << c.mSkills;
os << "Spells: " << std::hex << c.mSpells << std::dec << "\n";
os << "Unknown: " << std::hex << c.mUnknown << std::dec << "\n";
os << "Unknown2: " << std::hex << c.mUnknown2 << std::dec << "\n";
os << c.mConditions << "\n";
os << "SkillAffectors: " << c.mSkillAffectors << "\n";
os << "Inventory: " << c.mInventory << "\n";
return os;
}

}
