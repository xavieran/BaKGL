#pragma once

#include "bak/coordinates.hpp"
#include "bak/fmap.hpp"
#include "bak/layout.hpp"

#include "gui/clickButton.hpp"
#include "gui/townLabel.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class GameState;
}

namespace Gui {

class IGuiManager;

class Backgrounds;
class Font;
class Icons;
class TickAnimator;

class FullMap : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_FMAP.DAT";

    static constexpr auto sExitWidget = 0;

    FullMap(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);
    
    void DisplayMapMode();
    void DisplayGameStartMode(BAK::Chapter chapter, BAK::MapLocation location);
    void UpdateLocation();

private:
    void SetPlayerLocation(
        BAK::ZoneNumber zone,
        BAK::GamePositionAndHeading location);
    void SetPlayerLocation(BAK::MapLocation location);
    void DisplayGameStart(BAK::Chapter chapter);
    void StartPlayerPositionFlasher();
    void UpdatePlayerPositionIcon();
    void AddChildren();

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::FMapXY mFMapXY;
    BAK::FMapTowns mFMapTowns;

    BAK::Layout mLayout;

    ClickButton mExitButton;
    Button mPopup;
    TextBox mPopupText;

    Widget mPlayerLocation;
    std::vector<TownLabel> mTowns{};
    unsigned mPlayerPositionBaseIcon{};
    unsigned mPlayerPositionIconOffset{};
    int mPlayerPositionIconPulseDirection{1};
    TickAnimator* mPlayerPositionFlasher{};

    bool mGameStartScreenMode{};

    const Logging::Logger& mLogger;
};

}
