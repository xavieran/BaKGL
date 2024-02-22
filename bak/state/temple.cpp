#include "bak/state/temple.hpp"

#include "bak/state/event.hpp"

#include "bak/gameState.hpp"

namespace BAK::State {

static constexpr auto sTempleSeenFlag = 0x1950;

bool ReadTempleSeen(const GameState& gs, unsigned temple)
{
    return gs.ReadEventBool(sTempleSeenFlag + temple);
}

void SetTempleSeen(FileBuffer& fb, unsigned temple)
{
    SetEventFlagTrue(fb, sTempleSeenFlag + temple);
}

}
