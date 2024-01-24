#pragma once

#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

namespace BAK {

// Tracks the time in the game world
struct Time
{
    unsigned GetDays() const
    {
        return GetHours() / 24;
    }

    unsigned GetHour() const
    {
        return GetHours() % 24;
    }

    unsigned GetHours() const
    {
        return GetSeconds() / 3600;
    }

    unsigned GetMinutes() const
    {
        return GetSeconds() / 60;
    }

    unsigned GetMinute() const
    {
        return GetMinutes() % 60;
    }

    unsigned GetSeconds() const
    {
        return mTime * 2;
    }

    Time operator*(const Time& lhs) const
    {
        return Time(mTime * lhs.mTime);
    }

    Time operator/(const Time& lhs) const
    {
        return Time(mTime / lhs.mTime);
    }

    std::uint32_t mTime;
};

std::string ToString(Time t);

std::ostream& operator<<(std::ostream&, const Time&);

class WorldClock
{
public:
    // 0x0000 0000 is 0AM
    // 0x0001 0000 is 12AM
    // 0x0001 0800 is 1AM
    // 1 hour = 1800 ticks
    // Each tick is 2 seconds
    // One step forward is 0x1e ticks (60 seconds)
    Time mTime;
    Time mTimeLastSlept;
};

std::ostream& operator<<(std::ostream&, const WorldClock&);

}
