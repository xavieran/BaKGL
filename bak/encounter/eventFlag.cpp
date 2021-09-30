#include "bak/encounter/eventFlag.hpp"

#include <ios>

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const EventFlag& ef)
{
    os << "EventFlag { " << std::hex << ef.mEventPointer << std::dec
        << " Chance: " << +ef.mPercentChance
        << "% Enable: [" << ef.mIsEnable<< "]}";
    return os;
}

}
