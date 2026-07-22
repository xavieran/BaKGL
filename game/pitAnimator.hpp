#pragma once

#include "gui/IAnimator.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <optional>

namespace Game {

class PitAnimator : public Gui::IAnimator
{
public:
    PitAnimator(
        double duration,
        glm::vec3 start,
        glm::vec3 end,
        glm::vec3 pitCenter,
        float origHeight,
        float drop,
        std::function<bool(glm::vec2)>&& isOnPit,
        std::function<void(const glm::vec3&)>&& setPosition,
        std::function<void()>&& finished);

    void OnTimeDelta(double delta) override;
    bool IsAlive() const override;

private:
    glm::vec3 GetPosition(float t);

    bool mAlive{true};
    double mDuration;
    double mAccumulated{0};
    glm::vec3 mStart;
    glm::vec3 mEnd;
    glm::vec3 mPitCenter;
    float mOrigHeight;
    float mDrop;
    mutable std::optional<float> mReferenceRadius{};
    std::function<bool(glm::vec2)> mIsOnPit;
    std::function<void(const glm::vec3&)> mSetPosition;
    std::function<void()> mFinished;
    bool mEnteredPit{};
};

}
