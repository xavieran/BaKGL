#pragma once

#include "bak/layout.hpp"

#include "gui/core/widget.hpp"
#include "gui/clickButton.hpp"

#include <glm/glm.hpp>

namespace Gui {

class IGuiManager;
class Backgrounds;
class Font;

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
        LeavePreferencesFn&& leavePreferenceFn);
private:
    void AddChildren();

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    LeavePreferencesFn mLeavePreferencesFn;

    Widget mFrame;
    ClickButton mOk;
    ClickButton mCancel;
    ClickButton mDefaults;
};

}
