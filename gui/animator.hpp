#pragma once

#include "gui/IAnimator.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace Gui {

class LinearAnimator : public IAnimator
{
public:
    // every 10ms
    static constexpr auto mTickFrequency = .01;

    LinearAnimator(
        double duration,
        glm::vec4 begin,
        glm::vec4 end,
        std::function<bool(glm::vec4)>&& callback,
        std::function<void()>&& finished);

    ~LinearAnimator();

    void OnTimeDelta(double delta) override;
    bool IsAlive() const override;

private:
    bool mAlive;
    double mAccumulatedTimeDelta;
    double mDuration;
    double mTrueDuration;
    glm::vec4 mDelta;
    std::function<bool(glm::vec4)> mCallback;
    std::function<void()> mFinished;
};

}
