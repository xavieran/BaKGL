#pragma once

#include "com/ostream.hpp"
#include "bak/skills.hpp"
#include "bak/inventory.hpp"

#include <ostream>
#include <ios>
#include <string>

namespace BAK {

class Character
{
public:
    std::string mName;
    Skills mSkills;
    std::array<std::uint8_t, 6> mSpells;
    std::array<std::uint8_t, 2> mUnknown;
    std::array<std::uint8_t, 7> mUnknown2;
    Inventory mInventory;
};

std::ostream& operator<<(std::ostream&, const Character&);

}
