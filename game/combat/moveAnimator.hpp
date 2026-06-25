#pragma once

#include "game/combat/actorStore.hpp"

#include "gui/IAnimator.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace Game::Combat {

class MoveAnimator : public Gui::IAnimator
{
public:
    MoveAnimator(
        Actor& actor,
        glm::vec3 source,
        glm::vec3 target,
        double duration,
        std::function<void()>&& onFinished)
    :
        mAlive{true},
        mActor{actor},
        mSource{source},
        mTarget{target},
        mDuration{duration},
        mAccumulated{0},
        mOnFinished{std::move(onFinished)}
    {
        assert(mOnFinished);
    }

    void OnTimeDelta(double delta) override
    {
        if (!mAlive)
        {
            return;
        }

        mAccumulated += delta;
        auto progress = std::min(mAccumulated / mDuration, 1.0);
        mActor.mLocation = glm::mix(mSource, mTarget, static_cast<float>(progress));
        mActor.CalculateModelMatrix();

        if (progress >= 1.0)
        {

            mActor.mLocation = mTarget;
            mActor.CalculateModelMatrix();
            mAlive = false;
            mOnFinished();
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

private:
    bool mAlive;
    Actor& mActor;
    glm::vec3 mSource;
    glm::vec3 mTarget;
    double mDuration;
    double mAccumulated;
    std::function<void()> mOnFinished;
};

}
