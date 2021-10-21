#include "bak/container.hpp"

namespace BAK {

Container::Container(
    unsigned address,
    unsigned number,
    unsigned numberItems,
    unsigned capacity,
    unsigned type,
    Target dialog,
    glm::vec<2, unsigned> location,
    std::vector<InventoryItem>&& items)
:
    mAddress{address},
    mNumber{number},
    mNumberItems{numberItems},
    mCapacity{capacity},
    mType{type},
    mDialog{dialog},
    mLocation{location},
    mInventory{std::move(items)}
{}

std::ostream& operator<<(std::ostream& os, const Container& i)
{
    os << "Container { addr: " << std::hex << i.mAddress << std::dec 
        << ", num: " << i.mNumber << ", numItems: " << i.mNumberItems << ", capacity: " 
        << i.mCapacity << ", type: " << std::hex << i.mType << std::dec
        << ", dialog:" << i.mDialog << ", loc: " << i.mLocation
        << ", inventory: [" << i.mInventory << "]}";
    return os;
}

}
