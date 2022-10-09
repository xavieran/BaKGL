#pragma once

#include "audio/audio.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/colors.hpp"
#include "gui/widget.hpp"

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
        mLogger{Logging::LogState::GetLogger("Gui::FadeScreen")}
    {
    }

    [[nodiscard]] bool OnMouseEvent(const MouseEvent& event) override
    {
        return true;
    }

    void FadeIn(double duration)
    {
        mDuration = duration;

        SetColor(glm::vec4{0,0,0,0});
        mGuiManager.AddAnimator(
            LinearAnimator{
                mDuration / 2,
                glm::vec4{0, 0, 0, 0},
                glm::vec4{0, 0, 0, 1},
                [&](const auto& delta){
                    SetColor(GetDrawInfo().mColor + delta);
                    return false;
                },
                [&]{ mFadeInDone(); }
            });
    }

    void FadeOut()
    {
        SetColor(glm::vec4{0,0,0,1});
        mGuiManager.AddAnimator(
            LinearAnimator{
                mDuration / 2,
                glm::vec4{0, 0, 0, 1},
                glm::vec4{0, 0, 0, 0},
                [&](const auto& delta){
                    SetColor(GetDrawInfo().mColor + delta);
                    return false;
                },
                [&]{ mFadeOutDone(); }
            });
    }

private:
    IGuiManager& mGuiManager;
    DoneFunction mFadeInDone;
    DoneFunction mFadeOutDone;
    double mDuration;

    const Logging::Logger& mLogger;
};

}
