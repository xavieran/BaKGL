#pragma once

#include "audio/audio.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class ContentsScreen: public Widget
{
public:
    static constexpr auto sLayoutFile = "CONTENTS.DAT";
    static constexpr auto sBackground = "CONT2.SCX";

    static constexpr auto sExit = 9;

    using LeaveContentsFn = std::function<void()>;

    ContentsScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Font& font,
        LeaveContentsFn&& leaveContentsFn)
    :
        Widget{
            RectTag{},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            Color::black,
            false 
        },
        mGuiManager{guiManager},
        mFont{font},
        mBackgrounds{backgrounds},
        mLayout{sLayoutFile},
        mLeaveContentsFn{std::move(leaveContentsFn)},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
        },
        mExit{
            mLayout.GetWidgetLocation(sExit),
            mLayout.GetWidgetDimensions(sExit),
            mFont,
            "#Exit",
            [this]{ std::invoke(mLeaveContentsFn); }
        },
        mLogger{Logging::LogState::GetLogger("Gui::ContentsScreen")}
    {
        AddChildren();
    }

private:
    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);

        mExit.SetPosition(mLayout.GetWidgetLocation(sExit));

        AddChildBack(&mExit);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    LeaveContentsFn mLeaveContentsFn;

    Widget mFrame;
    ClickButton mExit;

    const Logging::Logger& mLogger;
};

}
