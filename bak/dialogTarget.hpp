#pragma once

#include "com/strongType.hpp"

#include <cstdint>
#include <functional>
#include <ostream>
#include <variant>

namespace BAK {

using KeyTarget = StrongType<std::uint32_t, struct KeyTargetTag>;

struct OffsetTarget
{
    std::uint8_t dialogFile;
    std::uint32_t value;

    bool operator==(const OffsetTarget other) const;
};

using Target = std::variant<KeyTarget, OffsetTarget>;

std::ostream& operator<<(std::ostream& os, const Target& t);

}

namespace std {

template<> struct hash<BAK::OffsetTarget>
{
	std::size_t operator()(const BAK::OffsetTarget& t) const noexcept;
};

}
