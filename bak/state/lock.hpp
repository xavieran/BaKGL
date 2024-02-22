#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK {
class GameState;
}

namespace BAK::State {

void SetLockHasBeenSeen(FileBuffer&, unsigned lockIndex);
bool CheckLockHasBeenSeen(const GameState&, unsigned lockIndex);

}

