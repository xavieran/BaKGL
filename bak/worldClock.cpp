#include "bak/worldClock.hpp"

#include <iomanip>
#include <sstream>

namespace BAK {

unsigned Time::GetDays() const
{
    return GetHours() / 24;
}

unsigned Time::GetHour() const
{
    return GetHours() % 24;
}

unsigned Time::GetHours() const
{
    return GetSeconds() / 3600;
}

unsigned Time::GetMinutes() const
{
    return GetSeconds() / 60;
}

unsigned Time::GetMinute() const
{
    return GetMinutes() % 60;
}

unsigned Time::GetSeconds() const
{
    return mTime * 2;
}

Time Time::operator+(const Time& lhs) const
{
    return Time(mTime + lhs.mTime);
}

Time& Time::operator+=(const Time& lhs)
{
    mTime += lhs.mTime;
    return *this;
}

Time Time::operator-(const Time& lhs) const
{
    return Time(mTime - lhs.mTime);
}

Time& Time::operator-=(const Time& lhs)
{
    mTime -= lhs.mTime;
    return *this;
}

Time Time::operator*(const Time& lhs) const
{
    return Time(mTime * lhs.mTime);
}

Time Time::operator/(const Time& lhs) const
{
    return Time(mTime / lhs.mTime);
}

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

WorldClock::WorldClock(Time time, Time timeLastSlept)
:
    mTime{time},
    mTimeLastSlept{timeLastSlept}
{
}

Time WorldClock::GetTime() const
{
    return mTime;
}

Time WorldClock::GetTimeLastSlept() const
{
    return mTimeLastSlept;
}

Time WorldClock::GetTimeSinceLastSlept() const
{
    return Time{mTime - mTimeLastSlept};
}

void WorldClock::AdvanceTime(Time timeDelta)
{
    mTime = mTime + timeDelta;
}

void WorldClock::SetTimeLastSlept(Time time)
{
    mTimeLastSlept = time;
}

void WorldClock::SetTime(Time time)
{
    mTime = time;
    mTimeLastSlept = time;
}

std::ostream& operator<<(std::ostream& os, const WorldClock& c)
{
    os << "Time: " << c.GetTime() << " LastSlept: " << c.GetTimeLastSlept();
    return os;
}

}
