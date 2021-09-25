#pragma once

#include <string>
#include <cstdint>

namespace BAK {

struct HotspotRef
{
    std::uint8_t mGdsNumber;
    char mGdsChar;

    bool operator==(const auto& rhs) const
    {
        return mGdsNumber == rhs.mGdsNumber
            && mGdsChar == rhs.mGdsChar;
    }

    bool operator!=(const auto& rhs)
    {
        return !(*this == rhs);
    }

    std::string ToString() const;
    std::string ToFilename() const;
};

char MakeHotspotChar(std::uint8_t);

std::ostream& operator<<(std::ostream&, const HotspotRef&);

}
