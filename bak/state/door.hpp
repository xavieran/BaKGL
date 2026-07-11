#pragma once

#include "bak/types.hpp"

namespace BAK {
class FileBuffer;
class GameState;
}

namespace BAK::State {

bool GetDoorState(const GameState&, DoorIndex doorIndex);
void SetDoorState(FileBuffer&, DoorIndex doorIndex, bool state);

}

