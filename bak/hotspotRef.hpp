#pragma once

#include <string>
#include <cstdint>

namespace BAK {

struct HotspotRef
{
    std::uint8_t mGdsNumber;
    char mGdsChar;

    bool operator==(const HotspotRef& rhs) const;
    bool operator!=(const HotspotRef& rhs);

    std::string ToString() const;
    std::string ToFilename() const;
};

char MakeHotspotChar(std::uint8_t);

std::ostream& operator<<(std::ostream&, const HotspotRef&);

}
