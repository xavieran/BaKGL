#include "bak/combat/combat.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/ostream.hpp"
#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include <sstream>

namespace BAK {

std::string_view ToString(CombatOutcome r)
{
    switch (r)
    {
        case CombatOutcome::None: return "None";
        case CombatOutcome::Won: return "Won";
        case CombatOutcome::Fled: return "Fled";
        case CombatOutcome::Dead: return "Dead";
    }
    return "Unknown";
}

std::string_view ToString(CombatantWorldState s)
{
    switch (s)
    {
        case CombatantWorldState::Invisible0: return "Invisible0";
        case CombatantWorldState::Invisible1: return "Invisible1";
        case CombatantWorldState::Moving0: return "Moving0";
        case CombatantWorldState::Moving1: return "Moving1";
        case CombatantWorldState::Dead: return "Dead";
    }
    return "Unknown";
}

std::ostream& operator<<(std::ostream& os, const CombatWorldLocation& cwl)
{
    os << "CombatWorldLocation{" << cwl.mPosition << " ImageIndex: " << +cwl.mImageIndex
        << " State: " << ToString(cwl.mState) << "}";
    return os;
}

bool CombatWorldLocation::IsLoaded() const
{
    return mPosition != GamePositionAndHeading{{}, {}}
        || mImageIndex != 0
        || mState != CombatantWorldState::Invisible1;
}

std::ostream& operator<<(std::ostream& os, const CombatantGridLocation& cgl)
{
    os << "CombatantGridLocation{ unk0: " << cgl.mUnknown0 << " Monster: "
        << cgl.mMonster << " pos: " << cgl.mGridPos
        << " unk1: " << cgl.mUnknown1
        << " state: " << +cgl.mState
        << " rest0: " << cgl.mRest0
        << " retreat: " << +cgl.mRetreatFactor
        << " rest1: " << cgl.mRest1<< "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CombatRelInfo & cri)
{
    os << "CombatRelInfo{ Combat# " << cri.mCombatIndex
        << " RelEnemy #" << cri.mCombatantRelativeIndex << "}";
    return os;
}

std::ostream& operator<<(std::ostream& os, const CombatEntityList& cel)
{
    os << "CombatEntityList{" << cel.mCombatants << "}";
    return os;
}

void LoadP1Dat()
{
    auto fb = FileBufferFactory::Get().CreateDataBuffer("P1.DAT");

    std::stringstream ss{};
    for (unsigned j = 0; j < 7; j++)
    {
        for (unsigned i = 0; i < 16; i++)
        {
            ss << " " << +fb.GetUint8();
        }
        Logging::LogDebug(__FUNCTION__) << "Skills? " << ss.str() << "\n";
        ss = {};
    }
    Logging::LogDebug(__FUNCTION__) << "Remaining: " << fb.GetBytesLeft() << "\n";
}

bool IsSpecialBattle(CombatIndex index)
{
    switch (index.mValue)
    {
        // These seem to be perpetual battles?
        case 151: [[fallthrough]];
        case 152: [[fallthrough]];
        case 235: [[fallthrough]];
        case 245: [[fallthrough]];
        case 291: [[fallthrough]];
        case 293: [[fallthrough]];
        case 335: [[fallthrough]];
        case 337: [[fallthrough]];
        case 338: [[fallthrough]];
        case 375: [[fallthrough]];
        case 410: [[fallthrough]];
        case 429: [[fallthrough]];
        case 430:
            return true;
        default:
            return false;
    }
}

}
