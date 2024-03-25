#include "bak/combat.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

#include <sstream>

namespace BAK {

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
