#include "bak/state/lock.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "com/logger.hpp"

namespace BAK::State {

void SetLockHasBeenSeen(FileBuffer& fb, unsigned lockIndex)
{
    SetEventFlagTrue(fb, sLockHasBeenSeenFlag + lockIndex);
}

bool CheckLockHasBeenSeen(FileBuffer& fb, unsigned lockIndex)
{
    return ReadEventBool(fb, sLockHasBeenSeenFlag + lockIndex);
}

}
