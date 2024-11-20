#include "bak/dialogTarget.hpp"

#include <iomanip>
#include <type_traits>

namespace BAK {

bool OffsetTarget::operator==(const OffsetTarget other) const
{
    return value == other.value 
        && dialogFile == other.dialogFile;
}

std::ostream& operator<<(std::ostream& os, const Target& t)
{
    std::visit([&os](auto&& arg){
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, KeyTarget>)
            os << "Key [ " << std::hex << arg.mValue << " ]";
        else
            os << "Offset { " << std::dec << +arg.dialogFile << " @ 0x" << std::hex << arg.value << " }";
    }, t);
    return os;
}

}
