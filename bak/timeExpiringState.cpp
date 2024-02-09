#include "bak/timeExpiringState.hpp"

namespace BAK {

std::ostream& operator<<(std::ostream& os, ExpiringStateType t)
{
    switch (t)
    {
        using enum ExpiringStateType;
        case None: return os << "None";
        case Light: return os << "Light";
        case Spell: return os << "Spell";
        case SetState: return os << "SetState";
        case ResetState: return os << "ResetState";
        default: return os << "UNKNOWN_TES!";
    }
}

std::ostream& operator<<(std::ostream& os, const TimeExpiringState& t)
{
    os << "TimeExpiringState{ Type: " << t.mType
        << " flags: " << std::hex << +t.mFlags
        << " data: " << t.mData << std::dec
        << " duration: " << t.mDuration << "}";
    return os;
}

TimeExpiringState* AddTimeExpiringState(
    std::vector<TimeExpiringState>& storage,
    ExpiringStateType type,
    std::uint16_t data,
    std::uint8_t flags,
    Time duration)
{
    if ((flags & 0x80) || (!(flags & 0x40)))
    {
        for (unsigned i = 0; i < storage.size(); i++)
        {
            auto& state = storage[i];
            if (state.mType == type && state.mData == data)
            {
                if (flags & 0x80)
                {
                    state.mDuration += duration;
                }
                else
                {
                    state.mDuration = duration;
                }
                return &state;
            }
        }
    }

    if (storage.size() >= 0x14)
    {
        return nullptr;
    }
    else
    {
        storage.emplace_back(TimeExpiringState{type, flags, data, duration});
        return &storage.back();
    }
}

TimeExpiringState* AddLightTimeExpiringState(std::vector<TimeExpiringState>& storage, unsigned stateType, Time duration) 
{
    // PaletteRequiresChange(); // so that we make sure to update the lighting
    return AddTimeExpiringState(storage, ExpiringStateType::Light, stateType, 0x80, duration);
}

TimeExpiringState* AddSpellTimeExpiringState(std::vector<TimeExpiringState>& storage, unsigned spell, Time duration) 
{
    // PaletteRequiresChange(); // so that we make sure to update the lighting
    return AddTimeExpiringState(storage, ExpiringStateType::Spell, static_cast<std::uint16_t>(spell), 0x80, duration);
}
}
