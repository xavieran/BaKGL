#include "gtest/gtest.h"

#include "bak/combat/calculations.hpp"
#include "bak/combat/types.hpp"
#include "bak/character.hpp"
#include "bak/condition.hpp"
#include "com/bits.hpp"
#include "com/random.hpp"
#include "bak/inventory.hpp"
#include "bak/inventoryItem.hpp"
#include "bak/monster.hpp"
#include "bak/objectInfo.hpp"
#include "bak/skills.hpp"
#include "bak/spells.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

#include <memory>
#include <vector>

namespace BAK {

struct CombatCalcFixture : public ::testing::Test
{
    static constexpr auto mStackSize = 10;

    std::vector<GameObject> mObjects;

    CombatCalcFixture()
    {
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);

        mObjects.emplace_back(GameObject{
            .mName = "Sword",
            .mFlags = 0x1000, // condition-based
            .mStrengthSwing = 15,
            .mAccuracySwing = 40,
            .mStrengthThrust = 20,
            .mAccuracyThrust = 50,
            .mRace = RacialModifier::None,
            .mType = ItemType::Sword});

        mObjects.emplace_back(GameObject{
            .mName = "Armor",
            .mFlags = 0x1000, // condition based
            .mStrengthSwing = 0,
            .mAccuracySwing = 10, // armor effectiveness
            .mStrengthThrust = 0,
            .mAccuracyThrust = 0,
            .mRace = RacialModifier::None,
            .mType = ItemType::Armor});
    }

    auto GetObject(const std::string& name)
    {
        return std::find_if(mObjects.begin(), mObjects.end(),
            [&](const auto& obj){ return obj.mName == name; });
    }

    InventoryItem MakeItem(
        const std::string& item,
        unsigned condition,
        std::uint8_t status = 0,
        std::uint8_t modifiers = 0)
    {
        auto it = GetObject(item);
        assert(it != mObjects.end());
        return InventoryItem{
            &(*it),
            ItemIndex{
                static_cast<std::uint8_t>(
                    std::distance(mObjects.begin(), it))},
            static_cast<std::uint8_t>(condition),
            status,
            modifiers};
    }

    Skills MakeSkills(
        unsigned strength = 10,
        unsigned melee = 25,
        unsigned defense = 15,
        unsigned health = 50)
    {
        Skills::SkillArray arr{};
        auto set = [&](SkillType st, unsigned val)
        {
            auto idx = static_cast<unsigned>(st);
            arr[idx] = Skill{
                .mMax = static_cast<std::uint8_t>(val),
                .mTrueSkill = static_cast<std::uint8_t>(val),
                .mCurrent = static_cast<std::uint8_t>(val),
                .mExperience = 0,
                .mModifier = 0,
                .mSelected = false,
                .mUnseenImprovement = false};
        };
        set(SkillType::Strength, strength);
        set(SkillType::Melee, melee);
        set(SkillType::Defense, defense);
        set(SkillType::Health, 100);
        set(SkillType::Stamina, 100);
        return Skills{arr, 0};
    }

    struct CharacterResources
    {
        std::unique_ptr<Inventory> mInventory;
        std::unique_ptr<Character> mCharacter;
    };

    CharacterResources MakeCharacter(
        const Skills& skills,
        std::vector<InventoryItem> items = {})
    {
        auto inv = std::make_unique<Inventory>(20, std::move(items));
        auto* invPtr = inv.get();

        auto charPtr = std::make_unique<Character>(
            0,
            "TestChar",
            skills,
            Spells{{}},
            std::array<std::uint8_t, 2>{},
            0,
            MonsterIndex{0},
            std::array<std::uint8_t, 6>{},
            Conditions{},
            invPtr,
            glm::uvec2{});

        return CharacterResources{
            .mInventory = std::move(inv),
            .mCharacter = std::move(charPtr)};
    }

    CharacterResources MakeCombatant(
        unsigned strength = 10,
        unsigned melee = 25,
        unsigned defense = 15,
        InventoryItem* weapon = nullptr,
        InventoryItem* armor = nullptr)
    {
        std::vector<InventoryItem> items;
        if (weapon)
        {
            weapon->SetEquipped(true);
            items.emplace_back(*weapon);
        }
        if (armor)
        {
            armor->SetEquipped(true);
            items.emplace_back(*armor);
        }
        return MakeCharacter(MakeSkills(strength, melee, defense), std::move(items));
    }
};

TEST_F(CombatCalcFixture, CalculateMeleeResult_Hit)
{
    auto attacker = MakeCombatant(10, 50, 0);
    auto defender = MakeCombatant(10, 0, 0);
    CombatState defenderState{};

    // We want hitRoll to be less than skill (which will be 50)
    Random::Get().SetReturn(40);
    auto result = CalculateMeleeResult(
        *attacker.mCharacter,
        *defender.mCharacter,
        defenderState,
        0);

    EXPECT_EQ(result, MeleeResult::Hit);
}

TEST_F(CombatCalcFixture, CalculateMeleeResult_Miss)
{
    auto attacker = MakeCombatant(10, 50, 0);
    auto defender = MakeCombatant(10, 0, 0);
    CombatState defenderState{};

    // We want hitRoll to be greater than skill (which will be 50)
    Random::Get().SetReturn(55);
    auto result = CalculateMeleeResult(
        *attacker.mCharacter,
        *defender.mCharacter,
        defenderState,
        0);

    EXPECT_EQ(result, MeleeResult::Miss);
}

TEST_F(CombatCalcFixture, CalculateMeleeResult_DefendingMiss)
{
    auto attacker = MakeCombatant(10, 50, 0);
    auto defender = MakeCombatant(10, 0, 50);
    CombatState defenderState{};
    defenderState.mIsDefending = true;

    // defending boosts hitRoll by 20 to 55
    Random::Get().SetReturn(35);
    auto result = CalculateMeleeResult(
        *attacker.mCharacter,
        *defender.mCharacter,
        defenderState,
        0);

    EXPECT_EQ(result, MeleeResult::Miss);
}

TEST_F(CombatCalcFixture, CalculateMeleeDamage_Base)
{
    auto weapon = MakeItem("Sword", 100);
    auto attacker = MakeCombatant(10, 0, 0, &weapon);
    auto defender = MakeCombatant(10, 0, 0);

    auto damage = CalculateMeleeDamage(
        *attacker.mCharacter,
        *defender.mCharacter,
        true);

    EXPECT_EQ(damage, 30);
}

TEST_F(CombatCalcFixture, CalculateMeleeDamage_WithWeapon)
{
    auto weapon = MakeItem("Sword", 50);
    auto attacker = MakeCombatant(10, 0, 0, &weapon);
    auto defender = MakeCombatant(10, 0, 0);

    auto damage = CalculateMeleeDamage(
        *attacker.mCharacter,
        *defender.mCharacter,
        true);

    EXPECT_EQ(damage, 20);
}

TEST_F(CombatCalcFixture, CalculateMeleeDamage_ThrustVsSwing)
{
    auto weapon = MakeItem("Sword", 100);
    auto attacker = MakeCombatant(5, 0, 0, &weapon);
    auto defender = MakeCombatant(5, 0, 0);

    auto thrustDmg = CalculateMeleeDamage(
        *attacker.mCharacter,
        *defender.mCharacter,
        true);

    auto swingDmg = CalculateMeleeDamage(
        *attacker.mCharacter,
        *defender.mCharacter,
        false);

    EXPECT_EQ(thrustDmg, 25);
    EXPECT_EQ(swingDmg, 20);
}

TEST_F(CombatCalcFixture, CalculateMeleeDamage_Floor1)
{
    auto weapon = MakeItem("Sword", 100);
    auto attacker = MakeCombatant(0, 0, 0, &weapon);
    auto defender = MakeCombatant(0, 0, 0);

    auto damage = CalculateMeleeDamage(
        *attacker.mCharacter,
        *defender.mCharacter,
        true);

    EXPECT_EQ(damage, 21);
}

TEST_F(CombatCalcFixture, CalculateParry_Base)
{
    auto defender = MakeCombatant(10, 0, 40);
    CombatState state{};

    auto parry = CalculateParry(*defender.mCharacter, state);

    EXPECT_EQ(parry, 10);
}

TEST_F(CombatCalcFixture, CalculateParry_ArmorBlessing)
{
    auto mods = SetBit(0, Modifier::Blessing1, true);
    auto armor = MakeItem("Armor", 100, 0, mods);
    auto defender = MakeCombatant(10, 0, 40, nullptr, &armor);
    CombatState state{};

    auto parry = CalculateParry(*defender.mCharacter, state);

    EXPECT_EQ(parry, 10);
}

TEST_F(CombatCalcFixture, CalculateParry_Clamp98)
{
    auto defender = MakeCombatant(10, 255, 255);
    CombatState state{};

    auto parry = CalculateParry(*defender.mCharacter, state);

    EXPECT_EQ(parry, 50);
}

TEST_F(CombatCalcFixture, CalculateArmorReduction_NoArmor)
{
    auto defender = MakeCombatant(10, 0, 20);
    auto reduction = CalculateArmorReduction(*defender.mCharacter);
    EXPECT_EQ(reduction, 0);
}

TEST_F(CombatCalcFixture, CalculateArmorReduction_WithArmor)
{
    auto armor = MakeItem("Armor", 50);
    auto defender = MakeCombatant(10, 0, 30, nullptr, &armor);

    auto reduction = CalculateArmorReduction(*defender.mCharacter);

    EXPECT_GE(reduction, 0);
    EXPECT_LE(reduction, 98);
}

TEST_F(CombatCalcFixture, GetMeleeModifierFlags_Base)
{
    auto weapon = MakeItem("Sword", 100);
    auto attacker = MakeCombatant(10, 0, 0, &weapon);

    auto flags = GetMeleeModifierFlags(*attacker.mCharacter);
    EXPECT_EQ(flags, 0x0580);
}

TEST_F(CombatCalcFixture, GetMeleeModifierFlags_Poison)
{
    auto weapon = MakeItem("Sword", 100, 0x80);
    auto attacker = MakeCombatant(10, 0, 0, &weapon);

    auto flags = GetMeleeModifierFlags(*attacker.mCharacter);
    EXPECT_TRUE(flags & std::to_underlying(ModifierFlags::Poison));
}

TEST_F(CombatCalcFixture, CalculateMonsterResistance_Table)
{
    auto damage = CalculateMonsterResistance(
        MonsterIndex{15}, 0, 100);

    EXPECT_EQ(damage, 50);
}

TEST_F(CombatCalcFixture, CalculateMonsterResistance_NoResistance)
{
    auto damage = CalculateMonsterResistance(
        MonsterIndex{0}, 0, 100);

    EXPECT_EQ(damage, 100);
}

TEST_F(CombatCalcFixture, CalculateMonsterWeakness_BrakNurr)
{
    auto damage = CalculateMonsterWeakness(
        sBrakNurr,
        std::to_underlying(ModifierFlags::Poison),
        100);

    EXPECT_EQ(damage, 150);
}

TEST_F(CombatCalcFixture, CalculateAccuracyBonus)
{
    auto bonus = CalculateAccuracyBonus(
        -1,
        50,
        100);

    EXPECT_EQ(bonus, 49);
}

TEST_F(CombatCalcFixture, CalculateBlessingEffect)
{
    auto blessed = CalculateBlessingEffect(
        100,
        {Modifier::Blessing2});

    EXPECT_EQ(blessed, 110);
}

TEST_F(CombatCalcFixture, CalculateArmorModReduction)
{
    auto reduced = CalculateArmorModReduction(
        Modifier::Flaming,
        {Modifier::Flaming},
        50);

    EXPECT_EQ(reduced, 0);
}

TEST_F(CombatCalcFixture, CalculateArmorModReduction_NoMatch)
{
    auto reduced = CalculateArmorModReduction(
        Modifier::Flaming,
        {Modifier::Frost},
        50);

    EXPECT_EQ(reduced, 50);
}

}
