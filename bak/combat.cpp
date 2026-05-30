#include "bak/combat.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/ostream.hpp"
#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include <sstream>

namespace BAK {

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

std::ostream& operator<<(std::ostream& os, const CombatantGridLocation& cgl)
{
    os << "CombatantGridLocation{ unk0: " << cgl.mUnknown0 << " Monster: "
        << cgl.mMonster << " pos: " << cgl.mGridPos
        << " unk1: " << cgl.mUnknown1
        << " unk2: " << +cgl.mUnknown2
        << " rest: " << cgl.mRest<< "}";
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

}
