#include "gui/tickAnimator.hpp"

#include <functional>

namespace Gui {

TickAnimator::TickAnimator(
    double tickSpeed,
    std::function<void()>&& callback)
:
    mTickSpeed{tickSpeed},
    mAccumulatedTimeDelta{0},
    mAlive{true},
    mCallback{std::move(callback)}
{
}

void TickAnimator::OnTimeDelta(double delta)
{
    mAccumulatedTimeDelta += delta;
    if (mAccumulatedTimeDelta > mTickSpeed && mAlive)
    {
        mAccumulatedTimeDelta = 0;
        mCallback();
    }
}

bool TickAnimator::IsAlive() const
{
    return mAlive;
}

void TickAnimator::Stop()
{
    mAlive = false;
}

}
