#include "gui/preferencesScreen.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

PreferencesScreen::PreferencesScreen(
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
    }
{
    AddChildren();
}

void PreferencesScreen::AddChildren()
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

}
