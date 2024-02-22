#include "bak/state/item.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

namespace BAK::State {

static constexpr auto maxItems = 0x14;

bool ReadItemHasBeenUsed(const GameState& gs, unsigned character, unsigned itemIndex) 
{
    return gs.ReadEventBool(
        sItemUsedForCharacterAtLeastOnce
            + ((character * maxItems) + itemIndex));
}

void SetItemHasBeenUsed(FileBuffer& fb, unsigned character, unsigned itemIndex)
{
    SetEventFlagTrue(
        fb,
        sItemUsedForCharacterAtLeastOnce
            + ((character * maxItems) + itemIndex));
}

}
