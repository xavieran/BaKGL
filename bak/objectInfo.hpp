#pragma once

#include "bak/types.hpp"

#include <array>
#include <ostream>
#include <string>

namespace BAK {

enum class Modifier
{
    Flaming      = 0,
    SteelFire    = 1,
    Frost        = 2,
    Enhancement1 = 3,
    Enhancement2 = 4,
    Blessing1    = 5,
    Blessing2    = 6,
    Blessing3    = 7
};

std::string_view ToString(Modifier);
std::ostream& operator<<(std::ostream& os, Modifier);

Modifier ToModifier(unsigned modifierMask);

enum class RacialModifier
{
    None,
    Tsurani,
    Elf,
    Human,
    Dwarf
};

std::string_view ToString(RacialModifier);

enum class SaleCategory : std::uint16_t
{
    Jewellery         = 0x0000,
    Utility           = 0x0001,
    Rations           = 0x0002,
    PreciousGems      = 0x0004,
    Keys              = 0x0008,
    All               = 0x0010, // I put this here...
    QuestItem         = 0x0020,
    UsableMundaneItem = 0x0040,
    Sword             = 0x0080,
    CrossbowRelated   = 0x0100,
    Armor             = 0x0200,
    UsableMagicalItem = 0x0400,
    Staff             = 0x0800,
    Scroll            = 0x1000,
    BookOrNote        = 0x2000,
    Potions           = 0x4000,
    Modifier          = 0x8000
};

std::string_view ToString(SaleCategory);
std::ostream& operator<<(std::ostream& os, SaleCategory cat);

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

std::vector<SaleCategory> GetCategories(std::uint16_t);

enum class ItemType
{
    Unspecified = 0,
    Sword = 1,
    Crossbow = 2,
    Staff = 3,
    Armor = 4,

    Key = 7,
    Tool = 8,
    WeaponOil = 9,
    ArmorOil = 0xa,
    SpecialOil = 0xb,
    Bowstring = 0xc,
    Scroll = 0xd,

    Note = 0x10,
    Book = 0x11,
    Potion = 0x12,
    Restoratives = 0x13,
    ConditionModifier = 0x14,
    Light = 0x15,
    Ingredient = 0x16,
    Ration = 0x17,
    Food = 0x18,
    Other = 0x19
};

std::string_view ToString(ItemType);

struct GameObject
{
    std::string mName{};
    unsigned int mFlags{};
    int mLevel{};
    int mValue{};
    int mStrengthSwing{};
    int mAccuracySwing{};
    int mStrengthThrust{};
    int mAccuracyThrust{};
    unsigned mImageIndex{};
    unsigned mImageSize{};
    unsigned mUseSound{};
    unsigned mSoundPlayTimes{};
    unsigned mStackSize{};
    unsigned mDefaultStackSize{};
    RacialModifier mRace{};
    std::uint16_t mCategories{};
    ItemType mType{};
    std::uint16_t mEffectMask{};
    std::int16_t mEffect{};
    std::uint16_t mPotionPowerOrBookChance{};
    std::uint16_t mAlternativeEffect{};
    std::uint16_t mModifierMask{};
    std::int16_t mModifier{};
    std::uint16_t mDullFactor0{};
    std::uint16_t mDullFactor1{};
    std::uint16_t mMinCondition{};
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
