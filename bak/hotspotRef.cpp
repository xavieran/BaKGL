#include "bak/hotspotRef.hpp"

#include <sstream>
#include <iomanip>

namespace BAK {

std::string HotspotRef::ToString() const
{
    std::stringstream ss{};
    ss << std::dec << +mGdsNumber << mGdsChar;
    return ss.str();
}

std::string HotspotRef::ToFilename() const
{
    return "GDS" + ToString() + ".DAT";
}

char MakeHotspotChar(std::uint8_t n)
{
    // This is kinda weird...
    if (n == 0) return 'A';
    return static_cast<char>(65 + n - 1);
}

std::ostream& operator<<(std::ostream& os, const HotspotRef& hr)
{
    os << hr.ToString();
    return os;
}

}
