#include "bak/state/temple.hpp"

#include "bak/state/event.hpp"

namespace BAK::State {

static constexpr auto sTempleSeenFlag = 0x1950;

bool ReadTempleSeen(FileBuffer& fb, unsigned temple)
{
    return ReadEventBool(fb, sTempleSeenFlag + temple);
}

void SetTempleSeen(FileBuffer& fb, unsigned temple)
{
    SetEventFlagTrue(fb, sTempleSeenFlag + temple);
}

}
