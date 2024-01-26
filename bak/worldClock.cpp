#include "bak/worldClock.hpp"

#include <sstream>

namespace BAK {

std::string ToString(Time t)
{
    std::stringstream ss{};
    ss << t.GetDays() << " days " << std::setw(2) << std::setfill('0')
        << t.GetHour() << ":" << std::setw(2) << std::setfill('0') << t.GetMinute()
        << " (" << std::hex << t.mTime << std::dec << ")";
    return ss.str();
}   

std::ostream& operator<<(std::ostream& os, const Time& t)
{
    os << "Time {" << ToString(t) << "}";
    return os;
}


std::ostream& operator<<(std::ostream& os, const WorldClock& c)
{
    os << "Time: " << c.GetTime() << " LastSlept: " << c.GetTimeLastSlept();
    return os;
}

}
