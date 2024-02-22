#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK {
class GameState;
}

namespace BAK::State {

void SetTempleSeen(FileBuffer&, unsigned temple);
bool ReadTempleSeen(const GameState&, unsigned temple);

}
