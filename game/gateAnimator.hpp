#pragma once

#include "gui/IAnimator.hpp"

#include "game/systems.hpp"

#include <vector>

namespace Game {

class GateAnimator : public Gui::IAnimator
{
    using FrameOffsets = std::vector<Graphics::MeshObjectStorage::OffsetAndLength>;
public:
    GateAnimator(
        Systems& systems,
        BAK::EntityIndex entityId,
        const FrameOffsets& frames,
        double timePerFrame)
    :
        mAlive{true},
        mSystems{systems},
        mEntityId{entityId},
        mFrames{frames},
        mCurrentFrame{0},
        mTimePerFrame{timePerFrame},
        mAccumulated{0}
    {
        mSystems.SetRenderableFrame(mEntityId, mFrames[mCurrentFrame]);
    }

    void OnTimeDelta(double delta) override
    {
        if (!mAlive)
            return;

        mAccumulated += delta;

        if (mAccumulated >= mTimePerFrame)
        {
            mAccumulated -= mTimePerFrame;
            mCurrentFrame = (mCurrentFrame + 1) % mFrames.size();
            mSystems.SetRenderableFrame(mEntityId, mFrames[mCurrentFrame]);
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

    void Stop()
    {
        mAlive = false;
    }

private:
    bool mAlive;
    Systems& mSystems;
    BAK::EntityIndex mEntityId;
    const FrameOffsets& mFrames;
    unsigned mCurrentFrame;
    double mTimePerFrame;
    double mAccumulated;
};

}
