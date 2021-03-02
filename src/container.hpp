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
    unsigned mAddress;
    glm::vec<2, std::uint16_t> mPair;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    glm::vec<2, unsigned> mLocation;
    std::vector<Item> mItems;
};

}
