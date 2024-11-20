#pragma once

#include "bak/money.hpp"
#include "bak/types.hpp"

namespace BAK {
class FileBuffer;
class GameState;
struct ShopStats;
}

namespace BAK::State {

void WritePartyMoney(FileBuffer&, Chapter, Royals);

Royals ReadPartyMoney(FileBuffer&, Chapter);

bool IsRomneyGuildWars(const GameState&, const ShopStats&);

}
