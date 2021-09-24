#pragma once

#include <functional>
#include <iomanip>
#include <type_traits>
#include <variant>

namespace BAK {

struct KeyTarget
{
    std::uint32_t value;

    bool operator==(const KeyTarget other) const
    {
        return value == other.value;
    }
};

struct OffsetTarget
{
    std::uint8_t dialogFile;
    std::uint32_t value;

    bool operator==(const OffsetTarget other) const
    {
        return value == other.value 
            && dialogFile == other.dialogFile;
    }
};



using Target = std::variant<KeyTarget, OffsetTarget>;

std::ostream& operator<<(std::ostream& os, const Target& t);

} // namespace BAK {

namespace std {

template<> struct hash<BAK::KeyTarget>
{
	std::size_t operator()(const BAK::KeyTarget& t) const noexcept
	{
		return t.value;
	}
};

template<> struct hash<BAK::OffsetTarget>
{
	std::size_t operator()(const BAK::OffsetTarget& t) const noexcept
	{
		return static_cast<std::size_t>(t.value) 
			+ ((static_cast<std::size_t>(t.dialogFile) << 32));
	}
};

} // namespace std
