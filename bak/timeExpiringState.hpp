#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

#include "bak/worldClock.hpp"

namespace BAK {

enum class ExpiringStateType : std::uint8_t
{
    None = 0,
    Light = 1,
    Spell = 2,
    SetState = 3,
    ResetState = 4
};

std::ostream& operator<<(std::ostream&, ExpiringStateType);

struct TimeExpiringState
{
    ExpiringStateType mType;
    std::uint8_t mFlags;
    std::uint16_t mData;
    Time mDuration;
};

std::ostream& operator<<(std::ostream&, const TimeExpiringState&);

TimeExpiringState* AddTimeExpiringState(
    std::vector<TimeExpiringState>& storage,
    ExpiringStateType type,
    std::uint16_t data,
    std::uint8_t flags,
    Time duration);

TimeExpiringState* AddLightTimeExpiringState(
    std::vector<TimeExpiringState>& storage,
    unsigned stateType,
    Time duration);

TimeExpiringState* AddSpellTimeExpiringState(
    std::vector<TimeExpiringState>& storage,
    unsigned spell,
    Time duration);
}
