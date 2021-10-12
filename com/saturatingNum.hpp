#pragma once

#include <limits>

template <
    typename T,
    T max = std::numeric_limits<T>::max(),
    T min = std::numeric_limits<T>::max()>
class SaturatingNum
{
public:
    explicit SaturatingNum(T value)
    :
        mValue{value}
    {}

    SaturatingNum<T>& operator+=(int rhs)
    {
        auto result = static_cast<unsigned>(mValue) + rhs;
        if (result > max)
        {
            mValue = max;
        }
        else if (result < std::numeric_limits<T>::min())
        {
            mValue = std::numeric_limits<T>::min();
        }
        else
        {
            mValue = result;
        }

        return *this;
    }

    SaturatingNum<T>& operator-=(int rhs)
    {
        return (*this) += (-rhs);
    }

    T Get() const
    {
        return mValue;
    }

private:
    T mValue;
};
