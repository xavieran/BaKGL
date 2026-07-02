#pragma once

#include "game/combat/actorStore.hpp"

#include "gui/IAnimator.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace Game::Combat {

class FlashAnimator : public Gui::IAnimator
{
public:
    static constexpr auto sDuration = 0.3;
    static constexpr auto sStartAlpha = 0.5f;

    FlashAnimator(
        Actor& actor,
        glm::vec4 color,
        double duration = sDuration,
        std::function<void()>&& onFinished = []{})
    :
        mAlive{true},
        mActor{actor},
        mColor{color},
        mDuration{duration},
        mAccumulated{0},
        mOnFinished{std::move(onFinished)}
    {
        mActor.mFlashColor = mColor;
    }

    void OnTimeDelta(double delta) override
    {
        if (!mAlive)
            return;

        mAccumulated += delta;
        auto progress = std::min(mAccumulated / mDuration, 1.0);
        mActor.mFlashColor->a = mColor.a * static_cast<float>(1.0 - progress);

        if (progress >= 1.0)
        {
            mActor.mFlashColor = std::nullopt;
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
    glm::vec4 mColor;
    double mDuration;
    double mAccumulated;
    std::function<void()> mOnFinished;
};

}
