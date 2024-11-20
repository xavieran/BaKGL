#pragma once

namespace BAK {
class FileBuffer;
class GameState;
}

namespace BAK::State {

bool ReadItemHasBeenUsed(const GameState&, unsigned character, unsigned itemIndex);
void SetItemHasBeenUsed(FileBuffer&, unsigned character, unsigned itemIndex);

}
