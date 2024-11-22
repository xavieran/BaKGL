#pragma once

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <functional>


namespace Gui {
class Icons;
class IGuiManager;
class TickAnimator;
}

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
        std::function<void(bool)>&& selected);

    void SetEnabledState(bool state);

public:
    void Reset();

    void SetSprite(unsigned sprite);
    void Entered();
    void Exited();

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
        std::function<void(unsigned)>&& callback);

    void Animate(std::pair<unsigned, unsigned> activeRange, IGuiManager& manager);
private:
    void HandleTickClicked(unsigned i);
    void Ready();
    void HandleTimeTick();

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
