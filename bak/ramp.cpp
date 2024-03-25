#include "bak/ramp.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const Ramp& ramp)
{
    os << "Ramp {\n";
    for (unsigned i = 0; i < ramp.mRamps.size(); i++)
    {
        os << "  " << i << " [";
        for (const auto& c : ramp.mRamps[i])
        {
            os << " " << +c;
        }
        os << "]\n";
    }
    os << "}";
    return os;
}

Ramp LoadRamp(FileBuffer& fb)
{
    Ramp ramp{};
    while (fb.GetBytesLeft() > 0)
    {
        std::array<std::uint8_t, 256> colors{};
        for (unsigned i = 0; i < 256; i++)
        {
            colors[i] = fb.GetUint8();
        }
        ramp.mRamps.emplace_back(colors);
    }
    return ramp;
}

}
