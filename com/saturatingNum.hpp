#pragma once

#include <limits>
#include <ostream>

template <
    typename T,
    T min = std::numeric_limits<T>::min(),
    T max = std::numeric_limits<T>::max()>
class SaturatingNum
{
public:
    using SelfType = SaturatingNum<T, min, max>;
    static constexpr auto sMin = min;
    static constexpr auto sMax = max;

    constexpr explicit SaturatingNum(T value) noexcept
    :
        mValue{value}
    {}

    constexpr SaturatingNum(SelfType&&) noexcept = default;
    constexpr SaturatingNum& operator=(SelfType&&) noexcept = default;
    constexpr SaturatingNum(const SelfType&) noexcept = default;
    constexpr SaturatingNum& operator=(const SelfType&) noexcept = default;

    constexpr SaturatingNum() noexcept
    :
        mValue{}
    {}

    SelfType& operator=(int rhs)
    {
        if (rhs > max)
        {
            mValue = max;
        }
        else if (rhs < min)
        {
            mValue = min;
        }
        else
        {
            mValue = rhs;
        }
        return *this;
    }

    SelfType& operator+=(int rhs)
    {
        const auto result = static_cast<int>(mValue) + rhs;
        *this = result;
        return *this;
    }

    SelfType& operator-=(int rhs)
    {
        return (*this) += (-rhs);
    }

    SelfType operator+(int rhs) const
    {
        SelfType result = static_cast<int>(mValue) + rhs;
        return result;
    }

    SelfType operator-(int rhs) const
    {
        return (*this) + (-rhs);
    }

    auto operator<=>(const SaturatingNum&) const = default;

    template <typename R>
    auto operator!=(R rhs) const
    {
        return mValue != rhs;
    }

    template <typename R>
    auto operator==(R rhs) const
    {
        return mValue == rhs;
    }

    T Get() const
    {
        return mValue;
    }

private:
    T mValue;
};

template <typename T, T min, T max>
std::ostream& operator<<(std::ostream& os, const SaturatingNum<T, min, max>& s)
{
    return os << +s.Get();
}


