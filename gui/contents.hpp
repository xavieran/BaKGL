#pragma once

#include "graphics/sprites.hpp"

#include "gui/clickButton.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/widget.hpp"

#include "bak/layout.hpp"
#include "bak/types.hpp"

#include <glm/glm.hpp>

namespace Gui {

class IGuiManager;
class Backgrounds;
class Font;

class ContentsScreen: public Widget
{
public:
    static constexpr auto sLayoutFile = "CONTENTS.DAT";
    static constexpr auto sBackground = "CONT2.SCX";
    //static constexpr auto sBackground = "CONTENTS.SCX";
    static constexpr auto sUnlockedChapters = "CONTENTS.SCX";

    static constexpr auto sExit = 9;

    using LeaveContentsFn = std::function<void()>;

    ContentsScreen(
        IGuiManager& guiManager,
        Graphics::SpriteManager& spriteManager,
        const Backgrounds& backgrounds,
        const Font& font,
        LeaveContentsFn&& leaveContentsFn);

    void SetUnlockedChapters(unsigned unlocked);

private:
    void AddChildren();
    void PlayChapter(BAK::Chapter chapter);
    void ShowTooltip(unsigned buttonIndex);
    void DismissTooltip();

    IGuiManager& mGuiManager;
    Graphics::SpriteManager::TemporarySpriteSheet mSpriteSheet;
    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    LeaveContentsFn mLeaveContentsFn;
    unsigned mUnlockedChapters;

    using ChapterButton = Clickable<
        Clickable<Widget, LeftMousePress, std::function<void()>>,
        RightMousePress,
        std::function<void()>>;
    Widget mFrame;
    std::vector<ChapterButton> mChapterButtons;
    ClickButton mExit;
    ClickButton mTooltipPopup;

    bool mShowingTooltip{false};

};

}
