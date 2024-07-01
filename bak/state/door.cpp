#include "bak/state/door.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

namespace BAK::State {

bool GetDoorState(const GameState& gs, unsigned doorIndex)
{
    return gs.ReadEventBool(sDoorFlag + doorIndex);
}

void SetDoorState(FileBuffer& fb, unsigned doorIndex, bool state)
{
    SetEventFlag(fb, sLockHasBeenSeenFlag + doorIndex, state);
}

}
