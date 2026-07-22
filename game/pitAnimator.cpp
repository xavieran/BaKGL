#include "game/pitAnimator.hpp"

#include "audio/audio.hpp"
#include "bak/sounds.hpp"
#include "com/assert.hpp"

#include <glm/geometric.hpp>

#include <limits>

namespace Game {

PitAnimator::PitAnimator(
    double duration,
    glm::vec3 start,
    glm::vec3 end,
    glm::vec3 pitCenter,
    float origHeight,
    float drop,
    std::function<bool(glm::vec2)>&& isOnPit,
    std::function<void(const glm::vec3&)>&& setPosition,
    std::function<void()>&& finished)
:
    mDuration{duration},
    mStart{start},
    mEnd{end},
    mPitCenter{pitCenter},
    mOrigHeight{origHeight},
    mDrop{drop},
    mIsOnPit{std::move(isOnPit)},
    mSetPosition{std::move(setPosition)},
    mFinished{std::move(finished)}
{
    ASSERT(mIsOnPit);
    ASSERT(mSetPosition);
    ASSERT(mFinished);
}

void PitAnimator::OnTimeDelta(double delta)
{
    mAccumulated += delta;

    const bool complete = mAccumulated >= mDuration;
    const auto t = complete ? 1.0f : static_cast<float>(mAccumulated / mDuration);

    auto position = GetPosition(t);
    mSetPosition(position);

    if (complete)
    {
        mAlive = false;
        mFinished();
    }
}

glm::vec3 PitAnimator::GetPosition(float t)
{
    const auto x = mStart.x + (mEnd.x - mStart.x) * t;
    const auto z = mStart.z + (mEnd.z - mStart.z) * t;

    auto position = glm::vec3{x, mStart.y, z};
    const auto xz = glm::vec2{x, z};

    if (!mIsOnPit(xz))
    {
        return position;
    }

    if (!mEnteredPit)
    {
        mEnteredPit = true;
        AudioA::GetAudioManager().PlaySound(AudioA::SoundIndex{BAK::sUseRope});
    }

    const auto distance = glm::distance(xz, glm::vec2{mPitCenter.x, mPitCenter.z});
    if (distance <= std::numeric_limits<float>::epsilon())
    {
        return position;
    }

    const auto referenceRadius = mReferenceRadius.value_or(distance);
    mReferenceRadius = referenceRadius;

    const float r = glm::clamp(distance / referenceRadius, 0.0f, 1.0f);
    position.y = (mOrigHeight - mDrop) + mDrop * (r * r);

    return position;
}

bool PitAnimator::IsAlive() const
{
    return mAlive;
}

}
