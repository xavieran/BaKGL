#include "bak/state/money.hpp"

#include "bak/gameState.hpp"
#include "bak/shop.hpp"
#include "bak/state/event.hpp"
#include "bak/types.hpp"

#include "com/logger.hpp"

namespace BAK::State {

constexpr std::uint32_t CalculateMoneyOffset(Chapter chapter)
{
    return 0x12f7 + ((chapter.mValue - 1) << 2) + 0x64;
}

void WritePartyMoney(
    FileBuffer& fb,
    Chapter chapter,
    Royals money)
{
    fb.Seek(CalculateMoneyOffset(chapter));
    fb.PutUint32LE(money.mValue);
}

Royals ReadPartyMoney(
    FileBuffer& fb,
    Chapter chapter)
{
    fb.Seek(CalculateMoneyOffset(chapter));
    return Royals{fb.GetUint32LE()};
}

bool IsRomneyGuildWars(const GameState& gs, const ShopStats& shop)
{
    const auto dc29 = gs.Apply(ReadEvent, 0xdc29);
    const auto dc2a = gs.Apply(ReadEvent, 0xdc2a);
    if (shop.mTempleNumber == 2 && gs.GetZone() == ZoneNumber{3})
    {
        if (!(dc29 & 0x1))
        {
            return false;
        }
        else
        {
            if (dc2a & 0x1)
            {
                return false;
            }
            return true;
        }
    }
    return false;
}


}
