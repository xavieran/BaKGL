#include "bak/state/lock.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

namespace BAK::State {

void SetLockHasBeenSeen(FileBuffer& fb, unsigned lockIndex)
{
    SetEventFlagTrue(fb, sLockHasBeenSeenFlag + lockIndex);
}

bool CheckLockHasBeenSeen(const GameState& gs, unsigned lockIndex)
{
    return gs.ReadEventBool(sLockHasBeenSeenFlag + lockIndex);
}

}
