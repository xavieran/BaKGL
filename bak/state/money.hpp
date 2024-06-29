#pragma once

#include "bak/file/fileBuffer.hpp"
#include "bak/money.hpp"
#include "bak/types.hpp"

namespace BAK {
class GameState;
class ShopStats;
}

namespace BAK::State {

void WritePartyMoney(FileBuffer&, Chapter, Royals);

Royals ReadPartyMoney(FileBuffer&, Chapter);

bool IsRomneyGuildWars(const GameState&, const ShopStats&);

}
