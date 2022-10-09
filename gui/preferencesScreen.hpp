#pragma once

#include "audio/audio.hpp"

#include "bak/IContainer.hpp"
#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class PreferencesScreen: public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_PREF.DAT";
    static constexpr auto sBackground = "OPTIONS2.SCX";

    static constexpr auto sOk = 0;
    static constexpr auto sCancel = 1;
    static constexpr auto sDefaults = 2;

    using LeavePreferencesFn = std::function<void()>;

    PreferencesScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Font& font,
        LeavePreferencesFn&& leavePreferenceFn)
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
        mLeavePreferencesFn{std::move(leavePreferenceFn)},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
        },
        mOk{
            mLayout.GetWidgetLocation(sOk),
            mLayout.GetWidgetDimensions(sOk),
            mFont,
            "#OK",
            [this]{ std::invoke(mLeavePreferencesFn); }
        },
        mCancel{
            mLayout.GetWidgetLocation(sCancel),
            mLayout.GetWidgetDimensions(sCancel),
            mFont,
            "#Cancel",
            [this]{ std::invoke(mLeavePreferencesFn); }
        },
        mDefaults{
            mLayout.GetWidgetLocation(sDefaults),
            mLayout.GetWidgetDimensions(sDefaults),
            mFont,
            "#Defaults",
            []{ }
        },
        mLogger{Logging::LogState::GetLogger("Gui::PreferencesScreen")}
    {
        AddChildren();
    }

private:
    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);

        mOk.SetPosition(mLayout.GetWidgetLocation(sOk));
        mCancel.SetPosition(mLayout.GetWidgetLocation(sCancel));
        mDefaults.SetPosition(mLayout.GetWidgetLocation(sDefaults));

        AddChildBack(&mOk);
        AddChildBack(&mCancel);
        AddChildBack(&mDefaults);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    LeavePreferencesFn mLeavePreferencesFn;

    Widget mFrame;
    ClickButton mOk;
    ClickButton mCancel;
    ClickButton mDefaults;

    const Logging::Logger& mLogger;
};

}
