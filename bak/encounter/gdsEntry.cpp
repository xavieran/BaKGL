#include "bak/encounter/gdsEntry.hpp"

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const GDSEntry& gds)
{
    os << "GDSEntry { " << gds.mHotspot.ToString() 
        << " Entry: " << gds.mEntryDialog 
        << " Exit: " << gds.mExitDialog 
        << " Exit: " << gds.mExitPosition
        << " Walk: [" << gds.mWalkToDest << "]}";
    return os;
}

}
