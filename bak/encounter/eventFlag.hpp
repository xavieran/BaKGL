#pragma once

#include <cassert>
#include <cstdint>
#include <ostream>
#include <vector>

namespace BAK::Encounter {

/*
1A 00 00 00
01 08 00 64 B5 DB 00 00
01 26 00 64 2B DB 00 00
01 26 00 64 2B DB 00 00
01 24 00 64 48 DB 00 00
01 04 00 64 48 DB 00 00
01 24 00 64 48 DB 00 00
01 24 00 64 00 00 00 00
01 20 00 64 66 DB 00 00
01 20 00 64 66 DB 00 00
01 20 00 64 66 DB 00 00
01 20 00 64 18 1D 00 00
01 10 00 64 37 DC 00 00
01 10 00 64 37 DC 00 00
01 10 00 64 37 DC 00 00
01 10 00 64 97 DB 00 00
01 10 00 64 97 DB 00 00
01 08 00 64 1F 1D 00 00
01 08 00 64 20 1D 00 00
01 08 00 64 21 1D 00 00
01 40 00 64 BA 1F 00 00
01 40 00 64 BA 1F 00 00
01 40 00 64 BA 1F 00 00
01 40 00 64 BA 1F 00 00
01 06 00 64 22 1D 00 00
01 06 00 64 22 1D 00 00
01 06 00 64 22 1D 00 00
*/
struct EventFlag
{
    std::uint8_t mPercentChance;
    std::uint16_t mEventPointer;
    bool mIsEnable;
};

std::ostream& operator<<(std::ostream& os, const EventFlag&);

template <bool isEnable>
class EventFlagFactory
{
public:
    static constexpr auto sEnable  = "DEF_ENAB.DAT";
    static constexpr auto sDisable = "DEF_DISA.DAT";

    EventFlagFactory();

    EventFlag Get(unsigned i) const;

private:
    void Load();

    std::vector<EventFlag> mEventFlags;
};

}
