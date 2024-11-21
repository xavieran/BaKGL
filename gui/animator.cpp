#include "gui/animator.hpp"

#include "com/assert.hpp"

#include <glm/glm.hpp>

namespace Gui {

LinearAnimator::LinearAnimator(
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

LinearAnimator::~LinearAnimator()
{
}

void LinearAnimator::OnTimeDelta(double delta)
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

bool LinearAnimator::IsAlive() const
{
    return mAlive;
}

}
