#pragma once

#include "bak/types.hpp"

#include <array>
#include <iostream>
#include <ostream>
#include <string>

namespace BAK {

enum class TemporaryModifier
{
    Fire      = 0x1,
    SteelFire = 0x2,
    Frost     = 0x4,
    Oil       = 0x8
};

enum class PermanentModifier
{
    BlueOil   = 0x1, // What is this?
    Blessing1 = 0x2,
    Blessing2 = 0x4,
    Blessing3 = 0x8
};

enum class RacialModifier
{
    None,
    Tsurani,
    Elf,
    Human,
    Dwarf
};

std::string_view ToString(RacialModifier);

enum class ItemCategory : std::uint16_t
{
    Inn,     // ale and brandy, etc.
    Key,
    Armor,
    NonSaleable,
    Other,
    Rations,
    Gemstones,
    Combat,  // potion noxum, powder bag, tuning fork
    Crossbow, 
    Sword, // include quarrel & bowstring
    Scroll, // excl. Valheru Armor
    Magical,
    Staff,    // excl. Crystal Staff
    Book,     // excl. Journals
    Potion,   // incl. Herbal Pack
    Modifier // incl. Coltari
};

std::string_view ToString(ItemCategory);
std::ostream& operator<<(std::ostream& os, ItemCategory cat);

std::vector<ItemCategory> GetCategories(std::uint16_t);

enum class ItemType
{
    Unspecified,
    Sword,
    Crossbow,
    Staff,
    Armor,
    Unknown5,
    Unknown6,
    Key,
    Tool,
    WeaponOil,
    ArmorOil,
    SpecialOil,
    Bowstring,
    Scroll,
    Unknown14,
    Unknown15,
    Note,
    Book,
    Potion,
    Restoratives,
    Container,
    Light,
    Ingredient,
    Ration,
    Food,
    Other
};

std::string_view ToString(ItemType);

class GameObject
{
public:
    std::string mName;
    unsigned int mFlags;
    int mLevel;
    int mValue;
    int mStrengthSwing;
    int mAccuracySwing;
    int mStrengthThrust;
    int mAccuracyThrust;
    unsigned mImageIndex;
    unsigned mImageSize;
    unsigned mStackSize;
    unsigned mDefaultStackSize;
    RacialModifier mRace;
    ItemType mType;
    std::uint16_t mEffectMask;
    std::int16_t mEffect;
    std::uint16_t mPotionPowerOrBookChance;
    std::uint16_t mModifierMask;
    std::int16_t mModifier;
    std::uint16_t mDullFactor0;
    std::uint16_t mDullFactor1;
    std::uint16_t mMinCondition;
};

std::ostream& operator<<(std::ostream&, const GameObject&);

class ObjectIndex
{
public:
    static constexpr auto sObjectCount = 0x8a;

    ObjectIndex();
    
    const GameObject& GetObject(ItemIndex) const;

    std::array<GameObject, sObjectCount> mObjects;
};

const ObjectIndex& GetObjectIndex();

std::ostream& operator<<(std::ostream&, const ObjectIndex&);

}
