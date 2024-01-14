#pragma once

template <typename T, typename U>
bool CheckBitSet(T value, U flag)
{
    const auto shifted = static_cast<T>(1) << static_cast<T>(flag);
    return (value & shifted) != 0;
}

template <typename T, typename U>
T SetBit(T status, U flag, bool state)
{
    if (state)
        return status | (static_cast<T>(1) << static_cast<T>(flag));
    else
        return status & (~(static_cast<T>(1) << static_cast<T>(flag)));
}
