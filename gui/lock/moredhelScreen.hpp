#pragma once

#include "bak/layout.hpp"

#include "gui/lock/tumbler.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/core/clickable.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class IContainer;
class GameState;
}

namespace Gui {
class IGuiManager;
class Backgrounds;
class Icons;
class Font;

class MoredhelScreen :
    public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_PUZL.DAT";
    static constexpr auto sBackground = "PUZZLE.SCX";

    // Request offsets
    static constexpr auto mExitRequest = 0;
    static constexpr auto mExitButton = 13;

    MoredhelScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& alienFont,
        const Font& puzzleFont,
        BAK::GameState& gameState);
    
    void ResetUnlocked();
    bool IsUnlocked() const;
    BAK::IContainer* GetContainer() const;
    void SetContainer(BAK::IContainer* container);

private:
    void RefreshGui();
    void UpdateTumblers();
    void IncrementTumbler(unsigned tumblerIndex);
    void EvaluateLock();
    void Unlocked();
    void CantOpenLock();
    void AddChildren();

    IGuiManager& mGuiManager;
    const Font& mAlienFont;
    const Font& mPuzzleFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    std::optional<BAK::FairyChest> mFairyChest;
    BAK::Layout mLayout;

    Widget mFrame;
    ClickButtonImage mExit;
    Widget mLeftClasp;
    Widget mRightClasp;
    TextBox mDescription;

    using ClickableTumbler = Clickable<
        Tumbler,
        LeftMousePress,
        std::function<void()>>;
    std::vector<ClickableTumbler> mTumblers;

    std::vector<TextBox> mReqLocs;
    
    BAK::IContainer* mContainer;
    bool mNeedRefresh;

    bool mUnlocked;
};

}
