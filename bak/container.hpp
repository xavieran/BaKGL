#pragma once

#include "graphics/glm.hpp"
#include <glm/glm.hpp>

#include "xbak/ObjectResource.h"

#include <ostream>

namespace BAK {

class Item
{
public:
    Item(
        unsigned itemNumber,
        const std::string& name,
        std::uint8_t condition,
        std::uint8_t modifiers);

    unsigned mItemNumber;
    std::string mName;
    std::uint8_t mCondition;
    std::uint8_t mModifiers;
};

std::ostream& operator<<(std::ostream& os, const Item& i);

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
        const std::vector<Item>& items);
 
    unsigned mAddress;
    unsigned mNumber;
    unsigned mNumberItems;
    unsigned mCapacity;
    unsigned mType;
    unsigned mIndex;
    glm::vec<2, unsigned> mLocation;
    std::vector<Item> mItems;
};

std::ostream& operator<<(std::ostream& os, const Container& i);

}
