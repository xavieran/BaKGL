#pragma once

#include <string>
#include <vector>

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

class Item
{
    std::string mName;
    unsigned int mFlags;
    int mLevel;
    int mValue;
    int mStrengthSwing;
    int mAccuracySwing;
    int mStrengthThrust;
    int mAccuracyThrust;
    // Amount of space item takes?
    unsigned mImageSize;
    RacialModifier mRace;
    ItemType mType;
    TemporaryModifier mEffect;
    PermanentModifier mModifier;
};

class InventoryItem
{
    int mCondition;
    Item mItem;
};

class Inventory
{
    std::vector<InventoryItem> mItems;
};

}
