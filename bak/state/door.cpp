#include "bak/state/door.hpp"

#include "bak/file/fileBuffer.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

namespace BAK::State {

bool GetDoorState(const GameState& gs, DoorIndex doorIndex)
{
    return gs.ReadEventBool(sDoorFlag + doorIndex.mValue);
}

void SetDoorState(FileBuffer& fb, DoorIndex doorIndex, bool state)
{
    SetEventFlag(fb, sDoorFlag + doorIndex.mValue, state);
}

}
