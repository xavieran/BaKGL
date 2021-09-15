#include "bak/character.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const Character& c)
{
    os << "Character [" << c.mName << " Skills: \n" << c.mSkills;
    os << "Spells: " << std::hex << c.mSpells << std::dec << "\n";
    os << "Unknown: " << std::hex << c.mUnknown << std::dec << "\n";
    os << "Unknown2: " << std::hex << c.mUnknown2 << std::dec << "\n";
    return os;
}

}
