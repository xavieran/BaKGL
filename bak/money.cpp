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

std::string ToShopString(Royals money)
{
    const auto sovereigns = BAK::GetSovereigns(money);
    const auto royals = BAK::GetRemainingRoyals(money);
    std::stringstream ss{};
    if (sovereigns.mValue != 0) ss << "#" << sovereigns << " gold";
    if (royals.mValue != 0) ss << " " << royals << " silver";
    return ss.str();
}

std::string ToShopDialogString(Royals money)
{
    const auto sovereigns = BAK::GetSovereigns(money);
    const auto royals = BAK::GetRemainingRoyals(money);
    std::stringstream ss{};
    if (sovereigns.mValue != 0) ss << '\xf0' << sovereigns << " \xf0" << "sovereign";
    if (sovereigns.mValue > 1) ss << "s";
    if (royals.mValue != 0 && sovereigns.mValue != 0) ss << " \xf0" << "and ";
    if (royals.mValue != 0) ss << '\xf0' << royals << " \xf0" << "royal";
    if (royals.mValue > 1) ss << "s";
    return ss.str();
}

}
