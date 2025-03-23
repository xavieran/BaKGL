#pragma once

#include "bak/dialogTarget.hpp"
#include "bak/layout.hpp"
#include "bak/types.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/mouseEvent.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

namespace Logging {
class Logger;
};

namespace BAK {
class GameState;
}

namespace Gui {
class IGuiManager;
class Backgrounds;
class Icons;
}

namespace Gui::Combat {

class CombatScreen : public Widget
{
public:
    static constexpr auto sLayoutFile = "COMBAT.DAT";
    static constexpr auto sShootLayoutFile = "SHOOT.DAT";
    static constexpr auto sBackground = "CFRAME.SCX";

    // Request offsets
    static constexpr auto mContainerTypeRequest = 3;

    static constexpr auto mNextPageButton = 52;
    static constexpr auto mNextPageRequest = 4;

    static constexpr auto mCharacterRequest = 5;
    static constexpr auto mExitRequest = 0;
    static constexpr auto mExitButton = 13;

    CombatScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);

    void SetSelectedCharacter(
        BAK::ActiveCharIndex character);

    /* Widget */
    bool OnMouseEvent(const MouseEvent& event) override;

private:
    void StartDialog(BAK::Target target);
    void RefreshGui();

    void UpdatePartyMembers();
    
    void AddChildren();
    
private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    BAK::Layout mLayout;

    Widget mFrame;

    using CharacterButton = Clickable<
        Clickable<
            Widget,
            RightMousePress,
            std::function<void()>>,
        LeftMousePress,
        std::function<void()>>;

    std::vector<CharacterButton> mCharacters;
    ClickButtonImage mExit;

    std::optional<BAK::ActiveCharIndex> mSelectedCharacter;
    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
