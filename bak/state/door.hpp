#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK {
class GameState;
}

namespace BAK::State {

bool GetDoorState(const GameState&, unsigned doorIndex);
void SetDoorState(FileBuffer&, unsigned doorIndex, bool state);

}

