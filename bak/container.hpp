#pragma once

#include "glm.hpp"
#include <glm/glm.hpp>

#include "xbak/ObjectResource.h"

#include <ostream>

namespace BAK
{

class Item
{
public:
    Item(
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

    unsigned mItemNumber;
    std::string mName;
    std::uint8_t mCondition;
    std::uint8_t mModifiers;
};

std::ostream& operator<<(std::ostream& os, const Item& i)
{
    return os << i.mName << " cond/qty: " << +i.mCondition 
        << std::hex << " mods: " << +i.mModifiers << std::dec;
}

class Container
{
public:
    Container(
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
 
    unsigned mAddress;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    unsigned mIndex;
    glm::vec<2, unsigned> mLocation;
    std::vector<Item> mItems;
};

}
