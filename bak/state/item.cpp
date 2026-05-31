#include "bak/state/item.hpp"

#include "bak/file/fileBuffer.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

namespace BAK::State {

bool ReadItemHasBeenUsed(const GameState& gs, unsigned character, unsigned itemIndex) 
{
    return gs.ReadEventBool(
        sItemUsedForCharacterAtLeastOnce
            + ((character * sMaxTrackedItems) + itemIndex));
}

void SetItemHasBeenUsed(FileBuffer& fb, unsigned character, unsigned itemIndex)
{
    SetEventFlagTrue(
        fb,
        sItemUsedForCharacterAtLeastOnce
            + ((character * sMaxTrackedItems) + itemIndex));
}

}
