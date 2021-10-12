#pragma once

#include <functional>
#include <ostream>

template <typename UnderlyingType, typename StrongTag>
class StrongType
{
public:
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
