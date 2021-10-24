#pragma once

#include "com/assert.hpp"

#include <functional>
#include <ostream>

template <typename UnderlyingT, typename StrongTag>
class StrongType
{
public:
    using UnderlyingType = UnderlyingT;

    using ConcreteType = StrongType<UnderlyingType, StrongTag>;
    constexpr StrongType() noexcept : mValue{} {}
    constexpr explicit StrongType(UnderlyingType v) noexcept : mValue{v} {}

    constexpr StrongType(const ConcreteType&) noexcept = default;
    constexpr ConcreteType& operator=(const ConcreteType&) noexcept = default;

    constexpr StrongType(ConcreteType&&) noexcept = default;
    constexpr ConcreteType& operator=(ConcreteType&&) noexcept = default;

    constexpr auto operator<=>(const ConcreteType&) const = default;

    UnderlyingType mValue;
};

template <typename UnderlyingType, typename Tag>
std::ostream& operator<<(std::ostream& os, const StrongType<UnderlyingType, Tag>& s)
{
    return os << +s.mValue;
}

namespace std {

template<typename U, typename Tag>
struct hash<StrongType<U, Tag>>
{
	std::size_t operator()(const StrongType<U, Tag>& t) const noexcept
	{
		return std::hash<U>{}(t.mValue);
	}
};

}

// [min, max)
template <
    typename StrongT,
    typename StrongT::UnderlyingType min,
    typename StrongT::UnderlyingType max>
class Bounded : public StrongT
{
public:
    using ConcreteType = Bounded<StrongT, min, max>;
    using typename StrongT::UnderlyingType;
    using StrongT::operator<=>;

    constexpr explicit Bounded(UnderlyingType v) noexcept
    :
        StrongT{v}
    {
        ASSERT(v >= min && v < max);
    }

    constexpr Bounded() noexcept : StrongT{min} {}

    constexpr Bounded(const ConcreteType&) noexcept = default;
    constexpr ConcreteType& operator=(const ConcreteType&) noexcept = default;

    constexpr Bounded(ConcreteType&&) noexcept = default;
    constexpr ConcreteType& operator=(ConcreteType&&) noexcept = default;


};

