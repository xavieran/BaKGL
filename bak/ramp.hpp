#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <vector>

#include "bak/file/fileBuffer.hpp"

namespace BAK {

class Ramp
{
public:
    std::vector<std::array<std::uint8_t, 256>> mRamps;
};

std::ostream& operator<<(std::ostream&, const Ramp&);

Ramp LoadRamp(FileBuffer&);

}
