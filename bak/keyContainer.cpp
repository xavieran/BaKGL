#include "bak/keyContainer.hpp"

namespace BAK {

KeyContainer::KeyContainer(
    Inventory&& inventory)
:
    mInventory{std::move(inventory)}
{}

std::ostream& operator<<(std::ostream& os, const KeyContainer& i)
{
    os << "KeyContainer {" << i.mInventory << "}";
    return os;
}

}
