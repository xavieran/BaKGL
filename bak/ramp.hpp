#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <vector>

namespace BAK {

class FileBuffer;

// These are used to determine the "fog" effect on sprites in
// the world based on their distance. The palette is modified
// by these at steps of 7 for zones.
class Ramp
{
public:
    std::vector<std::array<std::uint8_t, 256>> mRamps;
};

std::ostream& operator<<(std::ostream&, const Ramp&);

Ramp LoadRamp(FileBuffer&);

}
