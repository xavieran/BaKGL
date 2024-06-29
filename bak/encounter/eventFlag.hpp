#pragma once

#include "com/assert.hpp"

#include <cstdint>
#include <ostream>
#include <vector>

namespace BAK::Encounter {

struct EventFlag
{
    EventFlag(
        std::uint8_t percentChance,
        std::uint16_t eventPointer,
        bool isEnable)
    :
        mPercentChance{percentChance},
        mEventPointer{eventPointer},
        mIsEnable{isEnable}
    {}

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
