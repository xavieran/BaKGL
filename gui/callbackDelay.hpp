#pragma once

#include "gui/IAnimator.hpp"

#include <functional>

namespace Gui {

class CallbackDelay : public IAnimator
{
public:
    CallbackDelay(
        std::function<void()>&& callback,
        double delay);

    void OnTimeDelta(double delta) override;
    bool IsAlive() const override;

    bool mAlive;
    double mDelay;
    std::function<void()> mCallback;
};

}
