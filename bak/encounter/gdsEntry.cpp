#include "bak/encounter/gdsEntry.hpp"

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const GDSEntry& gds)
{
    os << "GDSEntry { " << gds.mHotspot.ToString() 
        << " Entry: " << std::hex << gds.mEntryDialog 
        << " Exit: " << gds.mExitDialog  << std::dec
        << " Exit: " << gds.mExitPosition
        << " Walk: [" << gds.mWalkToDest << "]}";
    return os;
}

}
