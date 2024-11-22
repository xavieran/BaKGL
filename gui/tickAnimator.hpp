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
        std::function<void()>&& callback);

    void OnTimeDelta(double delta) override;
    bool IsAlive() const override;
    void Stop();
private:
    double mTickSpeed;
    double mAccumulatedTimeDelta;
    bool mAlive;
    std::function<void()> mCallback;
};

}
