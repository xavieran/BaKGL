#include "gui/fadeScreen.hpp"

#include "com/logger.hpp"

#include "gui/animatorStore.hpp"
#include "gui/animator.hpp"
#include "gui/colors.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

FadeScreen::FadeScreen(
    AnimatorStore& animatorStore,
    DoneFunction&& fadeInDone,
    DoneFunction&& fadeOutDone)
:
    Widget{
        RectTag{},
        glm::vec2{0, 0},
        glm::vec2{320, 200},
        Color::black,
        false 
    },
    mAnimatorStore{animatorStore},
    mFadeInDone{std::move(fadeInDone)},
    mFadeOutDone{std::move(fadeOutDone)},
    mDuration{0.0},
    mFading{false},
    mLogger{Logging::LogState::GetLogger("Gui::FadeScreen")}
{
}

[[nodiscard]] bool FadeScreen::OnMouseEvent(const MouseEvent& event)
{
    mLogger.Debug() << "Got mouse event: " << event << "\n";
    return true;
}

void FadeScreen::FadeIn(double duration)
{
    ASSERT(!mFading);
    mFading = true;
    mDuration = duration;

    SetColor(glm::vec4{0,0,0,0});
    mAnimatorStore.AddAnimator(
        std::make_unique<LinearAnimator>(
            mDuration / 2,
            glm::vec4{0, 0, 0, 0},
            glm::vec4{0, 0, 0, 1},
            [&](const auto& delta){
                SetColor(GetDrawInfo().mColor + delta);
                return false;
            },
            [&]{ 
                mFading = false;
                mFadeInDone(); 
            }
        ));
}

void FadeScreen::FadeOut()
{
    ASSERT(!mFading);
    mFading = true;
    SetColor(glm::vec4{0,0,0,1});
    mAnimatorStore.AddAnimator(
        std::make_unique<LinearAnimator>(
            mDuration / 2,
            glm::vec4{0, 0, 0, 1},
            glm::vec4{0, 0, 0, 0},
            [&](const auto& delta){
                SetColor(GetDrawInfo().mColor + delta);
                return false;
            },
            [&]{
                mFading = false;
                mFadeOutDone();
            }
        ));
}

}

