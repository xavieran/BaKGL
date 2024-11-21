#pragma once

#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Logging {
class Logger;
}

namespace Gui {

class AnimatorStore;

class FadeScreen: public Widget
{
public:
    using DoneFunction = std::function<void()>;

    FadeScreen(
        AnimatorStore& animatorStore,
        DoneFunction&& fadeInDone,
        DoneFunction&& fadeOutDone);
    
    [[nodiscard]] bool OnMouseEvent(const MouseEvent& event) override;

    void FadeIn(double duration);
    void FadeOut();

private:
    AnimatorStore& mAnimatorStore;
    DoneFunction mFadeInDone;
    DoneFunction mFadeOutDone;
    double mDuration;

    bool mFading;

    const Logging::Logger& mLogger;
};

}
