#include "bak/state/dialog.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

namespace BAK::State {

void SetEventDialogAction(FileBuffer& fb, const SetFlag& setFlag)
{
    if (setFlag.mEventPointer >= 0xdac0
        && setFlag.mEventPointer % 10 == 0)
    {
        const auto offset = std::get<0>(CalculateComplexEventOffset(setFlag.mEventPointer));
        fb.Seek(offset);
        const auto data = fb.GetUint8();
        const auto newData = ((data & setFlag.mEventMask) 
            | setFlag.mEventData)
            ^ setFlag.mAlwaysZero;
        fb.Seek(offset);

        Logging::LogSpam(__FUNCTION__) << std::hex <<
            " " << setFlag << " offset: " << offset 
            << " data[" << +data << "] new[" << +newData <<"]\n" << std::dec;
        fb.PutUint8(newData);
    }
    else
    {
        if (setFlag.mEventPointer != 0)
            SetEventFlag(fb, setFlag.mEventPointer, setFlag.mEventValue);

        // TREAT THIS AS UINT16_T !!! EVENT MASK + EVENT DATA
        if (setFlag.mEventMask != 0)
            SetEventFlag(fb, setFlag.mEventMask, setFlag.mEventValue);

        if (setFlag.mAlwaysZero != 0)
            SetEventFlag(fb, setFlag.mAlwaysZero, setFlag.mEventValue);
    }
}

bool ReadConversationItemClicked(const GameState& gs, unsigned eventPtr) 
{
    return gs.ReadEventBool(sConversationChoiceMarkedFlag + eventPtr);
}

void SetConversationItemClicked(FileBuffer& fb, unsigned eventPtr)
{
    return SetEventFlagTrue(fb, sConversationChoiceMarkedFlag + eventPtr);
}

bool CheckConversationOptionInhibited(const GameState& gs, unsigned eventPtr)
{
    return gs.ReadEventBool(sConversationOptionInhibitedFlag + eventPtr);
}

}
