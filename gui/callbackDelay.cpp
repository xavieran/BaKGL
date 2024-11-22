#include "gui/callbackDelay.hpp"

namespace Gui {

CallbackDelay::CallbackDelay(
    std::function<void()>&& callback,
    double delay)
:
    mAlive{true},
    mDelay{delay},
    mCallback{std::move(callback)}
{
}

void CallbackDelay::OnTimeDelta(double delta) 
{
    mDelay -= delta;
    if (mDelay <= 0)
    {
        mCallback();
        mAlive = false;
    }
}

bool CallbackDelay::IsAlive() const
{
    return mAlive;
}

}
