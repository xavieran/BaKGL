#pragma once

#include "gui/IAnimator.hpp"

#include <functional>

namespace Gui {

// Calls the callback every "tickSpeed" seconds, until stopped.
class TickAnimator : public IAnimator
{
public:
    TickAnimator(
        double tickSpeed,
        std::function<void()>&& callback)
    :
        mTickSpeed{tickSpeed},
        mAccumulatedTimeDelta{0},
        mAlive{true},
        mCallback{std::move(callback)}
    {
    }

    void OnTimeDelta(double delta) override
    {
        mAccumulatedTimeDelta += delta;
        if (mAccumulatedTimeDelta > mTickSpeed && mAlive)
        {
            mAccumulatedTimeDelta = 0;
            mCallback();
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

    void Stop()
    {
        mAlive = false;
    }

private:
    double mTickSpeed;
    double mAccumulatedTimeDelta;
    bool mAlive;
    std::function<void()> mCallback;
};

}
