#include "bak/container.hpp"

namespace BAK {

Item::Item(
    unsigned itemNumber,
    const std::string& name,
    std::uint8_t condition,
    std::uint8_t modifiers)
:
    mItemNumber{itemNumber},
    mName{name},
    mCondition{condition},
    mModifiers{modifiers}
{}

std::ostream& operator<<(std::ostream& os, const Item& i)
{
    return os << i.mName << " cond/qty: " << +i.mCondition 
        << std::hex << " mods: " << +i.mModifiers << std::dec;
}

Container::Container(
    unsigned address,
    unsigned number,
    unsigned numberItems,
    unsigned capacity,
    unsigned type,
    unsigned index,
    glm::vec<2, unsigned> location,
    const std::vector<Item>& items)
:
    mAddress{address},
    mNumber{number},
    mNumberItems{numberItems},
    mCapacity{capacity},
    mType{type},
    mIndex{index},
    mLocation{location},
    mItems{items}
{}

std::ostream& operator<<(std::ostream& os, const Container& i)
{
    os << "Container { addr: " << std::hex << i.mAddress << std::dec 
        << ", num: " << i.mNumber << ", numItems: " << i.mNumberItems << ", capacity: " 
        << i.mCapacity << ", type: " << std::hex << i.mType << std::dec
        << ", idx:" << i.mIndex << ", loc: " << i.mLocation
        << ", items: [";
    auto sep = ' ';
    for (const auto& i : i.mItems)
        os << sep << i;
    os << "]}";
    return os;
}
}
