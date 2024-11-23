#include "bak/encounter/combat.hpp"

#include "bak/encounter/combat.ipp"

#include "com/ostream.hpp"

namespace BAK::Encounter {

std::ostream& operator<<(std::ostream& os, const CombatantData& comb)
{
    os << "{ Monster: " << comb.mMonster << ", MoveType: " << comb.mMovementType
        << ", " << comb.mLocation << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Combat& comb)
{
    os << "Combat { #" << comb.mCombatIndex
        << " Entry: " << std::hex << comb.mEntryDialog 
        << " Scout: " << comb.mScoutDialog << std::dec
        << " \nTrap: " << comb.mTrap
        << " \nNorth: " << comb.mNorthRetreat
        << " \nWest: " << comb.mWestRetreat
        << " \nSouth: " << comb.mSouthRetreat
        << " \nEast: " << comb.mEastRetreat
        << " unknown: " << comb.mUnknown
        << " isAmbush: " << comb.mIsAmbush
        << " \nCombatants: [";
    for (const auto& combat : comb.mCombatants)
    {
        os << combat << ", ";
    }
    os << "]";
    return os;
}

template class GenericCombatFactory<false>;

}
