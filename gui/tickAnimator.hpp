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
    void Pause();
    void Continue();
private:
    double mTickSpeed;
    double mAccumulatedTimeDelta{0};
    bool mAlive{true};
    bool mPaused{false};
    std::function<void()> mCallback;
};

}
