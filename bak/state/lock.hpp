#pragma once

namespace BAK {
class FileBuffer;
class GameState;
}

namespace BAK::State {

void SetLockHasBeenSeen(FileBuffer&, unsigned lockIndex);
bool CheckLockHasBeenSeen(const GameState&, unsigned lockIndex);

}

