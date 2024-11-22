#pragma once

namespace BAK {
class GameState;
class FileBuffer;
}

namespace BAK::State {

void SetTempleSeen(FileBuffer&, unsigned temple);
bool ReadTempleSeen(const GameState&, unsigned temple);

}
