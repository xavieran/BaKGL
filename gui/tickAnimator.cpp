#include "gui/tickAnimator.hpp"

#include <functional>

namespace Gui {

TickAnimator::TickAnimator(
    double tickSpeed,
    std::function<void()>&& callback)
:
    mTickSpeed{tickSpeed},
    mCallback{std::move(callback)}
{
}

void TickAnimator::OnTimeDelta(double delta)
{
    if (mPaused)
    {
        return;
    }

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

void TickAnimator::Continue()
{
    mPaused = false;
}

void TickAnimator::Stop()
{
    mAlive = false;
}

void TickAnimator::Pause()
{
    mPaused = true;
}

}
