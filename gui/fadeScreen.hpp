#pragma once

#include "audio/audio.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/animator.hpp"
#include "gui/colors.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class FadeScreen: public Widget
{
public:
    using DoneFunction = std::function<void()>;

    FadeScreen(
        IGuiManager& guiManager,
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
        mGuiManager{guiManager},
        mFadeInDone{std::move(fadeInDone)},
        mFadeOutDone{std::move(fadeOutDone)},
        mDuration{0.0},
        mFading{false},
        mLogger{Logging::LogState::GetLogger("Gui::FadeScreen")}
    {
    }

    [[nodiscard]] bool OnMouseEvent(const MouseEvent& event) override
    {
        mLogger.Debug() << "Got mouse event: " << event << "\n";
        return true;
    }

    void FadeIn(double duration)
    {
        ASSERT(!mFading);
        mFading = true;
        mDuration = duration;

        SetColor(glm::vec4{0,0,0,0});
        mGuiManager.AddAnimator(
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

    void FadeOut()
    {
        ASSERT(!mFading);
        mFading = true;
        SetColor(glm::vec4{0,0,0,1});
        mGuiManager.AddAnimator(
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

private:
    IGuiManager& mGuiManager;
    DoneFunction mFadeInDone;
    DoneFunction mFadeOutDone;
    double mDuration;

    bool mFading;

    const Logging::Logger& mLogger;
};

}
