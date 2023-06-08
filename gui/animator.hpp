#pragma once

#include "gui/IAnimator.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

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
        std::function<void()>&& finished)
    :
        mAlive{true},
        mAccumulatedTimeDelta{0},
        mDuration{duration},
        mTrueDuration{duration},
        mDelta{(end - begin)},
        mCallback{std::move(callback)},
        mFinished{std::move(finished)}
    {
        ASSERT(mCallback);
        ASSERT(mFinished);
    }

    ~LinearAnimator()
    {
    }

    void OnTimeDelta(double delta) override
    {
        mAccumulatedTimeDelta += delta;
        mDuration -= delta;

        bool finishEarly = false;
        if (mAccumulatedTimeDelta > mTickFrequency)
        {
            mAccumulatedTimeDelta -= mTickFrequency;
            finishEarly = mCallback(mDelta * static_cast<float>(delta / mTrueDuration));//mDelta);
        }

        if (finishEarly || mDuration < 0)
        {
            mAlive = false;
            mFinished();
        }
    }

    bool IsAlive() const override { return mAlive; }

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
