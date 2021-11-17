#include "bak/objectInfo.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "xbak/FileBuffer.h"

#include <sstream>
#include <iomanip>

namespace BAK {

std::string_view ToString(Modifier m)
{
    switch (m)
    {
    case Modifier::Flaming: return "Flaming";
    case Modifier::SteelFire: return "SteelFire";
    case Modifier::Frost: return "Frost";
    case Modifier::BrownOil: return "BrownOil";
    case Modifier::BlueOil: return "BlueOil";
    case Modifier::Blessing1: return "Blessing1";
    case Modifier::Blessing2: return "Blessing2";
    case Modifier::Blessing3: return "Blessing3";
    default: return "Unknown Modifier";
    }
}

std::ostream& operator<<(std::ostream& os, Modifier m)
{
    os << ToString(m);
    return os;
}

std::string_view ToString(RacialModifier m)
{
    switch (m)
    {
    case RacialModifier::None: return "None";
    case RacialModifier::Tsurani: return "Tsurani";
    case RacialModifier::Elf: return "Elf";
    case RacialModifier::Human: return "Human";
    case RacialModifier::Dwarf: return "Dwarf";
    default: return "Unknown RacialMod";
    }
}

std::string_view ToString(ItemCategory i)
{
    switch (i)
    {
    case ItemCategory::Modifier: return "Modifier";
    case ItemCategory::Potion: return "Potion";
    case ItemCategory::Book: return "Book";
    case ItemCategory::Staff: return "Staff";
    case ItemCategory::Magical: return "Magical";
    case ItemCategory::Armor: return "Armor";
    case ItemCategory::Crossbow: return "Crossbow";
    case ItemCategory::Sword: return "Sword";
    case ItemCategory::Combat: return "Combat";
    case ItemCategory::Gemstones: return "Gemstones";
    case ItemCategory::Rations: return "Rations";
    case ItemCategory::Other: return "Other";
    case ItemCategory::NonSaleable: return "NonSaleable";
    case ItemCategory::Scroll: return "Scroll";
    case ItemCategory::Key: return "Key";
    case ItemCategory::Inn: return "Inn";
    default: return "UnknownItemCategory";
    }
}

std::ostream& operator<<(std::ostream& os, ItemCategory cat)
{
    os << ToString(cat);
    return os;
}

std::vector<ItemCategory> GetCategories(std::uint16_t value)
{
    auto categories = std::vector<ItemCategory>{};
    for (std::uint16_t cat = 0; cat < 16; cat++)
    {
        if (((1 << cat) & value) != 0)
            categories.emplace_back(static_cast<ItemCategory>(cat));
    }
    return categories;
}

std::string_view ToString(ItemType i)
{
    switch (i)
    {
    case ItemType::Unspecified: return "Unspecified";
    case ItemType::Sword: return "Sword";
    case ItemType::Crossbow: return "Crossbow";
    case ItemType::Staff: return "Staff";
    case ItemType::Armor: return "Armor";
    case ItemType::Unknown5: return "Unknown5";
    case ItemType::Unknown6: return "Unknown6";
    case ItemType::Key: return "Key";
    case ItemType::Tool: return "Tool";
    case ItemType::WeaponOil: return "WeaponOil";
    case ItemType::ArmorOil: return "ArmorOil";
    case ItemType::SpecialOil: return "SpecialOil";
    case ItemType::Bowstring: return "Bowstring";
    case ItemType::Scroll: return "Scroll";
    case ItemType::Unknown14: return "Unknown14";
    case ItemType::Unknown15: return "Unknown15";
    case ItemType::Note: return "Note";
    case ItemType::Book: return "Book";
    case ItemType::Potion: return "Potion";
    case ItemType::Restoratives: return "Restoratives";
    case ItemType::Container: return "Container";
    case ItemType::Light: return "Light";
    case ItemType::Ingredient: return "Ingredient";
    case ItemType::Ration: return "Ration";
    case ItemType::Food: return "Food";
    case ItemType::Other: return "Other";
    default: return "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const GameObject& go)
{
    os << "Object: { " << go.mName << std::hex << " fl: " << go.mFlags << std::dec
        << " itm: " << go.mImageIndex
        << " lvl: " << go.mLevel << " val: " << go.mValue
        << " swing (" << go.mStrengthSwing << ", " << go.mAccuracySwing << ")"
        << " thrust (" << go.mStrengthThrust << ", " << go.mAccuracyThrust << ")"
        << " size: " << go.mImageSize << " " << ToString(go.mRace) << " " << ToString(go.mType)
        << " stackSize: " << go.mStackSize << " defaultStackSize: " << go.mDefaultStackSize
        << " eff (" << std::hex << go.mEffectMask << ", " << std::dec << go.mEffect
        << " ) potionPower: " << go.mPotionPowerOrBookChance
        << " mod (" << std::hex << go.mModifierMask << ", " << std::dec << go.mModifier
        << ") df0: " << go.mDullFactor0 << " df1: " << go.mDullFactor1 
        << " minCond: " << go.mMinCondition << "}";
    return os;
}

ObjectIndex::ObjectIndex()
{
    const auto& logger = Logging::LogState::GetLogger("BAK::ObjectIndex");
    auto fb = FileBufferFactory::CreateFileBuffer("OBJINFO.DAT");
    for (unsigned i = 0; i < sObjectCount; i++)
    {
        const auto name = fb.GetString(30);
        logger.Debug() << "ItemOff: " << std::hex << fb.Tell() << std::dec << "\n";
        const auto unknown = fb.GetArray<2>();
        const auto flags = fb.GetUint16LE();
        const auto unknown2 = fb.GetArray<2>();
        const auto level = fb.GetSint16LE();
        const auto value = fb.GetSint16LE();
        const auto strengthSwing  = fb.GetSint16LE();
        const auto strengthThrust = fb.GetSint16LE();
        const auto accuracySwing = fb.GetSint16LE();
        const auto accuracyThrust = fb.GetSint16LE();
        const auto optionalImageIndex = fb.GetUint16LE();
        const auto imageSize = fb.GetUint16LE();
        const auto unknown3 = fb.GetArray<2>();
        const auto stackSize = fb.GetUint8();
        const auto defaultStackSize = fb.GetUint8();
        const auto race = fb.GetUint16LE();
        const auto unknown4 = fb.GetArray<2>();
        const auto type = fb.GetUint16LE();
        const auto effectMask = fb.GetUint16LE();
        const auto effect = fb.GetSint16LE();
        const auto potionPowerOrBookChance = fb.GetUint16LE();
        const auto unknown5 = fb.GetArray<2>();
        const auto modifierMask = fb.GetUint16LE();
        const auto modifier = fb.GetSint16LE();
        const auto dullFactor0 = fb.GetUint16LE();
        const auto dullFactor1 = fb.GetUint16LE();
        const auto minimumCondition = fb.GetUint16LE();

        logger.Debug() << i << std::hex << " Unknown: " << unknown << "|"
            << unknown2 << "|" << unknown3 << "|" << unknown4 << "|"
            << unknown5 << "|" << name << std::dec << "\n";

        mObjects[i] = GameObject{
            name,
            flags,
            level,
            value,
            strengthSwing,
            accuracySwing,
            strengthThrust,
            accuracyThrust,
            optionalImageIndex != 0 ? optionalImageIndex : i,
            imageSize,
            stackSize,
            defaultStackSize,
            static_cast<RacialModifier>(race),
            static_cast<ItemType>(type),
            effectMask,
            effect,
            potionPowerOrBookChance,
            modifierMask,
            modifier,
            dullFactor0,
            dullFactor1,
            minimumCondition};
        logger.Debug() << mObjects[i] << "\n";
    }
}
    
const GameObject& ObjectIndex::GetObject(ItemIndex index) const
{
    ASSERT(index.mValue < sObjectCount);
    return mObjects[index.mValue];
}

const ObjectIndex& GetObjectIndex()
{
    static auto objectIndex = ObjectIndex{};
    return objectIndex;
}

std::ostream& ShowRow(std::ostream& os, const GameObject& go)
{
    std::stringstream ss{};
    ss << std::setw(30) << std::setfill(' ');
    ss << go.mName;

    os << ss.str() << "\t" 
        << std::hex << std::setw(4) << std::setfill('0') << go.mFlags << std::dec
        << "\t" << std::setw(4) << std::setfill('0') << go.mLevel 
        << "\t" << std::setw(5) << std::setfill('0') << go.mValue
        << "\t(" << std::setw(2) << go.mStrengthSwing << 
            ", " << std::setw(2) << go.mAccuracySwing << ")"
        << "\t(" << std::setw(2) << go.mStrengthThrust << 
            ", " << std::setw(2) << go.mAccuracyThrust << ")"
        << "\t" << go.mImageSize << "\t" << ToString(go.mRace) << "\t" << ToString(go.mType)
        << "\t(" << std::hex << go.mEffectMask << ", " << std::dec << go.mEffect << ")"
        << "\t(" << std::hex << go.mModifierMask << ", " << std::dec << go.mModifier << ")\t";
    return os;
}

std::ostream& operator<<(std::ostream& os, const ObjectIndex& oi)
{
    std::stringstream ss{};
    ss << std::setw(30) << std::setfill(' ');
    ss << "Name";
    os << "Number\t" << ss.str() << "\t\tFlags\tLevel\tValue\tSwing\t\tThrust\t\tSize\tRace\tType\tEffect\tModifier\t\n";
    for(unsigned i = 0; i < oi.sObjectCount; i++)
    {
        std::stringstream ss{};
        ss << std::setw(3) << std::setfill('0');
        ss << i << " (" << std::hex;
        ss << std::setw(2) << std::setfill('0');
        ss << i << std::dec << ")\t";
        os << ss.str();
        ShowRow(os, oi.GetObject(ItemIndex{i})) << "\n";

    }
    return os;
}

}
