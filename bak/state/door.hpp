#pragma once

namespace BAK {
class FileBuffer;
class GameState;
}

namespace BAK::State {

bool GetDoorState(const GameState&, unsigned doorIndex);
void SetDoorState(FileBuffer&, unsigned doorIndex, bool state);

}

