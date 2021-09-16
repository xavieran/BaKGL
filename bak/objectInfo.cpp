#include "bak/objectInfo.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "xbak/FileBuffer.h"

#include <sstream>
#include <iomanip>
#include <ios>

namespace BAK {

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
    os << go.mName << std::hex << " fl: " << go.mFlags << std::dec
        << " lvl: " << go.mLevel << " val: " << go.mValue
        << " swing (" << go.mStrengthSwing << ", " << go.mAccuracySwing << ")"
        << " thrust (" << go.mStrengthThrust << ", " << go.mAccuracyThrust << ")"
        << " size: " << go.mImageSize << " " << ToString(go.mRace) << " " << ToString(go.mType)
        << " eff (" << std::hex << go.mEffectMask << ", " << std::dec << go.mEffect
        << " mod (" << std::hex << go.mModifierMask << ", " << std::dec << go.mModifier;
    return os;
}

ObjectIndex::ObjectIndex()
{
    const auto& logger = Logging::LogState::GetLogger("BAK::ObjectIndex");
    auto fb = FileBufferFactory::CreateFileBuffer("OBJINFO.DAT");
    for (unsigned i = 0; i < sObjectCount; i++)
    {
        const auto name = fb.GetString(30);
        const auto unknown = fb.GetArray<2>();
        const auto flags = fb.GetUint16LE();
        const auto unknown2 = fb.GetArray<2>();
        const auto level = fb.GetSint16LE();
        const auto value = fb.GetSint16LE();
        const auto strengthSwing  = fb.GetSint16LE();
        const auto strengthThrust = fb.GetSint16LE();
        const auto accuracySwing = fb.GetSint16LE();
        const auto accuracyThrust = fb.GetSint16LE();
        const auto unknown3 = fb.GetArray<2>();
        const auto imageSize = fb.GetUint16LE();
        const auto unknown4 = fb.GetArray<4>();
        const auto race = fb.GetUint16LE();
        const auto unknown5 = fb.GetArray<2>();
        const auto type = fb.GetUint16LE();
        const auto effectMask = fb.GetUint16LE();
        const auto effect = fb.GetSint16LE();
        const auto unknown6 = fb.GetArray<4>();
        const auto modifierMask = fb.GetUint16LE();
        const auto modifier = fb.GetSint16LE();
        const auto unknown7 = fb.GetArray<6>();

        logger.Debug() << "Item: " << i << " Unknown: " << unknown << " "
            << unknown2 << " " << unknown3 << " " << unknown4 << " "
            << unknown5 << " " << unknown6 << " " << unknown7 << "\n";

        mObjects[i] = GameObject{
            name,
            flags,
            level,
            value,
            strengthSwing,
            accuracySwing,
            strengthThrust,
            accuracyThrust,
            imageSize,
            static_cast<RacialModifier>(race),
            static_cast<ItemType>(type),
            effectMask,
            effect,
            modifierMask,
            modifier};
    }
}
    
const GameObject& ObjectIndex::GetObject(IndexType index) const
{
    assert(index < sObjectCount);
    return mObjects[index];
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
        ShowRow(os, oi.GetObject(i)) << "\n";

    }
    return os;
}

}
