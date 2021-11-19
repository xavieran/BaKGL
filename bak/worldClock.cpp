#include "bak/worldClock.hpp"

namespace BAK {

std::string ToString(Time t)
{
    std::stringstream ss{};
    ss << t.GetDays() << " days " << std::setw(2) << std::setfill('0')
        << t.GetHour() << ":" << t.GetMinute();
    return ss.str();
}   


std::ostream& operator<<(std::ostream& os, const Time& t)
{
    os << "Time {" << ToString(t) << "}";
    return os;
}


std::ostream& operator<<(std::ostream& os, const WorldClock& c)
{
    os << "Time: " << c.mTime << " LastSlept: " << c.mTimeLastSlept;
    return os;
}

}
