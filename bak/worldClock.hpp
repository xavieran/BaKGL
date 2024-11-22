#pragma once

#include <cstdint>
#include <ostream>

namespace BAK {

// Tracks the time in the game world
struct Time
{
    unsigned GetDays() const;
    unsigned GetHour() const;
    unsigned GetHours() const;
    unsigned GetMinutes() const;
    unsigned GetMinute() const;
    unsigned GetSeconds() const;

    constexpr auto operator<=>(const Time&) const = default;

    Time operator+(const Time& lhs) const;
    Time& operator+=(const Time& lhs);
    Time operator-(const Time& lhs) const;
    Time& operator-=(const Time& lhs);
    Time operator*(const Time& lhs) const;
    Time operator/(const Time& lhs) const;

    template <typename Numeric> requires std::is_integral_v<Numeric>
    Time operator*(Numeric lhs) const
    {
        return Time(mTime * lhs);
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
    WorldClock(Time time, Time timeLastSlept);

    Time GetTime() const;
    Time GetTimeLastSlept() const;
    Time GetTimeSinceLastSlept() const;
    void AdvanceTime(Time timeDelta);
    void SetTimeLastSlept(Time time);
    void SetTime(Time time);

private:
    Time mTime;
    Time mTimeLastSlept;
};

std::ostream& operator<<(std::ostream&, const WorldClock&);

}
