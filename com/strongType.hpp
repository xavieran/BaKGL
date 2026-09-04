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

    auto operator<=>(const ConcreteType&) const = default;

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

// [Min, Max)
template <
    typename StrongT,
    typename StrongT::UnderlyingType Min,
    typename StrongT::UnderlyingType Max>
class Bounded : public StrongT
{
public:
    using ConcreteType = Bounded<StrongT, Min, Max>;
    using typename StrongT::UnderlyingType;

    constexpr explicit Bounded(UnderlyingType v) noexcept
    :
        StrongT{v}
    {
        ASSERT(v >= Min && v < Max);
    }

    auto operator<=>(const ConcreteType&) const = default;

    constexpr Bounded() noexcept : StrongT{Min} {}

    constexpr Bounded(const ConcreteType&) noexcept = default;
    constexpr ConcreteType& operator=(const ConcreteType&) noexcept = default;

    constexpr Bounded(ConcreteType&&) noexcept = default;
    constexpr ConcreteType& operator=(ConcreteType&&) noexcept = default;
};

namespace std {

template <
    typename StrongT,
    typename StrongT::UnderlyingType Min,
    typename StrongT::UnderlyingType Max>
struct hash<Bounded<StrongT, Min, Max>>
{
	std::size_t operator()(const Bounded<StrongT, Min, Max>& t) const noexcept
	{
		return std::hash<StrongT>{}(t);
	}
};

}

