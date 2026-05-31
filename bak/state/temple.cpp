#include "bak/state/temple.hpp"

#include "bak/file/fileBuffer.hpp"
#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

namespace BAK::State {

bool ReadTempleSeen(const GameState& gs, unsigned temple)
{
    return gs.ReadEventBool(sTempleSeenFlag + temple);
}

void SetTempleSeen(FileBuffer& fb, unsigned temple)
{
    SetEventFlagTrue(fb, sTempleSeenFlag + temple);
}

}
