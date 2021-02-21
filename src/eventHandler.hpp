#pragma once

#include "xbak/EventListener.h"

namespace BAK {

struct EventRouter : public KeyboardEventListener, LoopEventListener
{
    using KeyboardEventHandler = std::function<void(const KeyboardEvent&)>;
    using LoopEventHandler = std::function<void(const LoopEvent&)>;

    EventRouter(
        KeyboardEventHandler&& keyPressedHandler,
        KeyboardEventHandler&& keyReleasedHandler,
        LoopEventHandler&& loopEventHandler)
    :
        mKeyPressedHandler{keyPressedHandler},
        mKeyReleasedHandler{keyReleasedHandler},
        mLoopEventHandler{loopEventHandler}
    {
    }

    void KeyPressed(const KeyboardEvent& kbe) override
    {
        assert(mKeyPressedHandler);
        mKeyPressedHandler(kbe);
    }

    void KeyReleased(const KeyboardEvent& kbe) override
    {
        assert(mKeyReleasedHandler);
        mKeyReleasedHandler(kbe);
    }
    
    void LoopComplete(const LoopEvent& le) override
    {
        assert(mLoopEventHandler);
        mLoopEventHandler(le);
    }

    KeyboardEventHandler mKeyPressedHandler;
    KeyboardEventHandler mKeyReleasedHandler;
    LoopEventHandler mLoopEventHandler;
};

}
