#pragma once

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"
#include "gui/icons.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/tickAnimator.hpp"

#include "bak/spells.hpp"

namespace Gui::Cast {

namespace detail {

class PowerTick : public Widget
{
    static constexpr auto sLit = 1;
    static constexpr auto sUnlit = 2;
    static constexpr auto sMajorLit = 3;
    static constexpr auto sMajorUnlit = 4;
    static constexpr auto sHighlighted = 5;

public:
    PowerTick(
        const Icons& icons,
        glm::vec2 pos,
        unsigned index,
        std::function<void(bool)>&& selected)
    :
        Widget{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetCastIcon(sUnlit)),
            std::get<Graphics::TextureIndex>(icons.GetCastIcon(sUnlit)),
            pos,
            {8, 3},
            false
        },
        mIcons{icons},
        mIsMajor{((index + 1) % 5) == 0},
        mEnabled{false},
        mCallback{std::move(selected)}
    {
    }

    void SetEnabledState(bool state)
    {
        mEnabled = state;
        Reset();
    }

public:
    void Reset()
    {
        if (mIsMajor)
        {
            if (mEnabled)
            {
                SetSprite(sMajorLit);
            }
            else
            {
                SetSprite(sMajorUnlit);
            }
        }
        else
        {
            if (mEnabled)
            {
                SetSprite(sLit);
            }
            else
            {
                SetSprite(sUnlit);
            }
        }
    }

    void SetSprite(unsigned sprite)
    {
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetCastIcon(sprite)));
    }

    void Entered()
    {
        if (mEnabled)
        {
            SetSprite(sHighlighted);
            mCallback(true);
        }
    }

    void Exited()
    {
        if (mEnabled)
        {
            Reset();
            mCallback(false);
        }
    }

    const Icons& mIcons;
    bool mIsMajor;
    bool mEnabled;
    std::function<void(bool)> mCallback;
};

}

class PowerRing : public Widget
{
    static constexpr auto sTickSpeed = .005;
public:
    PowerRing(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(unsigned)>&& callback)
    :
        Widget{
            RectTag{},
            pos,
            glm::vec2{320, 200},
            glm::vec4{},
            true},
        mIcons{icons},
        mCurrentTick{},
        mActiveRange{std::make_pair(0, 0)},
        mAnimator{},
        mTicks{},
        mCallback{std::move(callback)}
    {
        const auto& points = BAK::PowerRing::Get().GetPoints();
        mTicks.reserve(points.size());
        for (unsigned i = 0; i < points.size(); i++)
        {
            mTicks.emplace_back(
                [this, i=i]{ HandleTickClicked(i); },
                mIcons,
                points[i] - glm::ivec2{1},
                i,
                [](bool){});
            mTicks.back().SetEnabledState(false);
        }
    }

    void Animate(std::pair<unsigned, unsigned> activeRange, IGuiManager& manager)
    {
        ClearChildren();

        mActiveRange = activeRange;
        mCurrentTick = 0;

        auto animator = std::make_unique<TickAnimator>(sTickSpeed, [&](){ HandleTimeTick(); });
        mAnimator = animator.get();
        manager.AddAnimator(std::move(animator));
    }

private:
    void HandleTickClicked(unsigned i)
    {
        mCallback(i);
    }

    void Ready()
    {
    }

    void HandleTimeTick()
    {
        if (mCurrentTick < mTicks.size())
        {
            mTicks[mCurrentTick].SetEnabledState(false);
            AddChildBack(&mTicks[(mTicks.size() - 1) - mCurrentTick]);
        }
        else if (mCurrentTick >= mTicks.size())
        {
            auto tick = mCurrentTick - mTicks.size();
            if (tick < mActiveRange.first)
            {
            }
            else if (tick >= mActiveRange.first && tick <= mActiveRange.second)
            {
                mTicks[tick].SetEnabledState(true);
            }
            else
            {
                mAnimator->Stop();
                Ready();
            }
        }

        mCurrentTick++;
    }

    using PowerTick = Highlightable<
        Clickable<
            detail::PowerTick,
            LeftMousePress,
            std::function<void()>>,
        true>;

    const Icons& mIcons;

    unsigned mCurrentTick;
    std::pair<unsigned, unsigned> mActiveRange;
    TickAnimator* mAnimator;

    std::vector<PowerTick> mTicks;
    std::function<void(unsigned)> mCallback;
};

}
