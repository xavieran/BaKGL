#pragma once

#include "game/combat/actorStore.hpp"

#include "gui/IAnimator.hpp"

#include <glm/glm.hpp>

#include <functional>


namespace Game::Combat {

class FrameAnimator : public Gui::IAnimator
{
public:
    FrameAnimator(
        Actor& actor,
        double timePerFrame,
        std::function<void()>&& onFinished)
    :
        mAlive{true},
        mActor{actor},
        mTimePerFrame{timePerFrame},
        mAccumulated{0},
        mOnFinished{std::move(onFinished)}
    {
        actor.mAnimating = true;
        assert(actor.mFrame == 0);
        assert(mOnFinished);
    }

    void OnTimeDelta(double delta) override
    {
        if (!mAlive)
        {
            return;
        }

        mAccumulated += delta;

        if (mAccumulated > mTimePerFrame)
        {
            mActor.mFrame += 1;
            mActor.Update();

            if (mActor.mFrame == 0)
            {
                mActor.mAnimating = false;
                mAlive = false;
                mOnFinished();
            }

            mAccumulated = 0.0;
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

private:
    bool mAlive;
    Actor& mActor;
    double mTimePerFrame;
    double mAccumulated;
    std::function<void()> mOnFinished;
};

}
