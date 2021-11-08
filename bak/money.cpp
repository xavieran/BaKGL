#include "bak/money.hpp"

#include <sstream>


namespace BAK {

Sovereigns GetSovereigns(Royals royals)
{
    return Sovereigns{royals.mValue / 10};
}

Royals GetRoyals(Sovereigns sovereigns)
{
    return Royals{sovereigns.mValue * 10};
}


Royals GetRemainingRoyals(Royals royals)
{
    return Royals{royals.mValue % 10};
}

std::string ToString(Royals money)
{
    const auto sovereigns = BAK::GetSovereigns(money);
    const auto royals = BAK::GetRemainingRoyals(money);
    std::stringstream ss{};
    ss << "#" << sovereigns << "s " << royals << "r";
    return ss.str();

}

}
