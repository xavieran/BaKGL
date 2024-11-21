#pragma once

#include "bak/coordinates.hpp"
#include "bak/layout.hpp"

#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class GameState;
}

namespace Logging {
class Logger;
}

namespace Gui {

class IGuiManager;

class Backgrounds;
class Icons;
class Font;

class MainView : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_MAIN.DAT";

    static constexpr auto sForward = 2;
    static constexpr auto sBackward = 1;
    static constexpr auto sSnapToRoad = 4;
    static constexpr auto sFullMap = 5;
    static constexpr auto sCast = 6;
    static constexpr auto sBookmark = 7;
    static constexpr auto sCamp = 8;
    static constexpr auto sMainMenu = 9;
    static constexpr auto sCharacterWidgetBegin = 10;

    MainView(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& spellFont);

    void SetHeading(BAK::GameHeading heading);
    void HandleButton(unsigned buttonIndex);
    void UpdatePartyMembers(const BAK::GameState& gameState);
    void ShowPortrait(BAK::ActiveCharIndex character);
    void ShowInventory(BAK::ActiveCharIndex character);
private:
    void AddChildren();

    IGuiManager& mGuiManager;
    const Icons& mIcons;
    const Font& mSpellFont;

    BAK::Layout mLayout;

    std::vector<Widget> mActiveSpells;
    Compass mCompass;
    std::vector<ClickButtonImage> mButtons;
    std::vector<ClickButtonImage> mCharacters;

    const Logging::Logger& mLogger;
};

}
