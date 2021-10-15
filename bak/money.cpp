#include "bak/money.hpp"


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

}
