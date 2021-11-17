#pragma once

template <typename T, typename U>
bool CheckBitSet(T value, U flag)
{
    const auto shifted = 1 << static_cast<T>(flag);
    return (value & shifted) != 0;
}

template <typename T, typename U>
T SetBit(T status, U flag, bool state)
{
    if (state)
        return status | (1 << static_cast<T>(flag));
    else
        return status & (~(1 << static_cast<T>(flag)));
}
