#include "gui/doubleClickStateMachine.hpp"

namespace Gui {

DoubleClickStateMachine::DoubleClickStateMachine(
    std::function<void(glm::vec2)>&& singlePress,
    std::function<void(glm::vec2)>&& singleRelease,
    std::function<void(glm::vec2)>&& doublePress)
:
    mSinglePress{std::move(singlePress)},
    mSingleRelease{std::move(singleRelease)},
    mDoublePress{std::move(doublePress)}
{}

void DoubleClickStateMachine::HandlePress(glm::vec2 clickPos, float time)
{
    if (mPress)
    {
        mDoublePress(clickPos);
        mPress.reset();
        mRelease.reset();
    }
    else
    {
        mRelease.reset();
        mPress = Event{time, clickPos};
    }
}

void DoubleClickStateMachine::HandleRelease(glm::vec2 clickPos, float time)
{
    if (!mPress)
    {
        mSingleRelease(clickPos);
    }
    else
    {
        mRelease = Event{time, clickPos};
    }
}

void DoubleClickStateMachine::UpdateTime(float time)
{
    if (mPress && ((time - mPress->mClickTime) > mReleaseTimeout))
    {
        if (!mRelease || (((time - mPress->mClickTime) > mDoubleClickTimeout)))
        {
            mSinglePress(mPress->mClickPos);
            if (mRelease)
            {
                mSingleRelease(mRelease->mClickPos);
            }
            mPress.reset();
            mRelease.reset();
        }
    }
}

}
