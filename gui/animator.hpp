#pragma once

#include "com/assert.hpp"
#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class LinearAnimator
{
public:
    // every 10ms
    static constexpr auto mTickFrequency = .02;

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
        mDelta{(end - begin) / static_cast<float>((duration / mTickFrequency))},
        mCallback{std::move(callback)},
        mFinished{std::move(finished)}
    {
        ASSERT(mCallback);
        ASSERT(mFinished);
        Logging::LogDebug("Gui::LinearAnimator") 
            << "Constructed @" << this << "\n";
    }

    ~LinearAnimator()
    {
        Logging::LogDebug("Gui::LinearAnimator") 
            << "Destructed @" << this << "\n";
    }

    void OnTimeDelta(double delta)
    {
        mAccumulatedTimeDelta += delta;
        mDuration -= delta;

        bool finishEarly = false;
        if (mAccumulatedTimeDelta > mTickFrequency)
        {
            Logging::LogSpam("Gui::LinearAnimator") << "Ticking : " << delta << "\n";
            mAccumulatedTimeDelta -= mTickFrequency;
            finishEarly = mCallback(mDelta);
        }

        if (finishEarly || mDuration < 0)
        {
            Logging::LogDebug("Gui::LinearAnimator") 
                << "Marking dead: " << this << "\n";
            mAlive = false;
            mFinished();
        }
    }

    bool IsAlive() const { return mAlive; }

private:
    bool mAlive;
    double mAccumulatedTimeDelta;
    double mDuration;
    glm::vec4 mDelta;
    std::function<bool(glm::vec4)> mCallback;
    std::function<void()> mFinished;
};

}
