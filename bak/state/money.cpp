#include "bak/state/money.hpp"

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
}
