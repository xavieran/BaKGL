#pragma once

#include "gui/IAnimator.hpp"

#include <functional>

namespace Gui {

class CallbackDelay : public IAnimator
{
public:
    CallbackDelay(
        std::function<void()>&& callback,
        double delay)
    :
        mAlive{true},
        mDelay{delay},
        mCallback{std::move(callback)}
    {
    }

    void OnTimeDelta(double delta) override
    {
        mDelay -= delta;
        if (mDelay <= 0)
        {
            mCallback();
            mAlive = false;
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

    bool mAlive;
    double mDelay;
    std::function<void()> mCallback;
};

}
