#include "bak/worldClock.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, const WorldClock& c)
{
    os << "Time: " << c.mTime.ToString() << " LastSlept: " << c.mTimeLastSlept.ToString();
    return os;
}

}
