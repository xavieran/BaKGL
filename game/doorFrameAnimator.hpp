#pragma once

#include "gui/IAnimator.hpp"

#include "game/systems.hpp"

#include <functional>
#include <vector>

namespace Game {

class DoorFrameAnimator : public Gui::IAnimator
{
    using FrameOffsets = std::vector<Graphics::MeshObjectStorage::OffsetAndLength>;
public:
    DoorFrameAnimator(
        Systems& systems,
        BAK::EntityIndex entityId,
        const FrameOffsets& frames,
        bool opening,
        double timePerFrame,
        std::function<void()>&& onFinished)
    :
        mAlive{true},
        mSystems{systems},
        mEntityId{entityId},
        mFrames{frames},
        mCurrentFrame{opening ? 0u : static_cast<unsigned>(mFrames.size() - 1)},
        mTargetFrame{opening ? static_cast<unsigned>(mFrames.size() - 1) : 0u},
        mStep{opening ? 1 : -1},
        mTimePerFrame{timePerFrame},
        mAccumulated{0},
        mOnFinished{std::move(onFinished)}
    {
        mSystems.SetRenderableFrame(mEntityId, mFrames[mCurrentFrame]);
    }

    void OnTimeDelta(double delta) override
    {
        if (!mAlive)
        {
            return;
        }

        mAccumulated += delta;

        if (mAccumulated >= mTimePerFrame)
        {
            mAccumulated -= mTimePerFrame;

            if (mCurrentFrame == mTargetFrame)
            {
                mAlive = false;
                if (mOnFinished)
                {
                    mOnFinished();
                }
                return;
            }

            mCurrentFrame += mStep;
            mSystems.SetRenderableFrame(mEntityId, mFrames[mCurrentFrame]);
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

private:
    bool mAlive;
    Systems& mSystems;
    BAK::EntityIndex mEntityId;
    const FrameOffsets& mFrames;
    unsigned mCurrentFrame;
    unsigned mTargetFrame;
    int mStep;
    double mTimePerFrame;
    double mAccumulated;
    std::function<void()> mOnFinished;
};

}
