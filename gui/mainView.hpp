#pragma once

#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/layout.hpp"

#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"
#include "gui/IMainView.hpp"

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

class MainView : public Widget, public IMainView
{
public:
    static constexpr auto sLayoutFile = "REQ_MAIN.DAT";
    static constexpr auto sMapLayoutFile = "REQ_MAP.DAT";

    static constexpr auto sBackward = 1;
    static constexpr auto sForward = 2;
    static constexpr auto sFollowRoad = 4;
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
        const Font& spellFont,
        const Font& gameFont);

    void SetHeading(BAK::GameHeading heading) override;
    void UpdatePartyMembers(const BAK::GameState& gameState);
    void SetCanSaveBookmark(bool can) override;
    void SetFollowRoadVisible(bool visible) override;
    void SetFollowRoadActive(bool active) override;
    void SetZoomOutVisible(bool visible) override;
    void SetZoomInVisible(bool visible) override;
    void SetInMapView(bool inMapView) override;
    [[nodiscard]] bool OnMouseEvent(const MouseEvent& event) override;
private:
    void AddChildren();

    void ShowInventory(BAK::ActiveCharIndex character);
    void ShowPortrait(BAK::ActiveCharIndex character);
    void HandleButton(unsigned buttonIndex);
    void HandleBookmark();
    void HandleCast();
    void HandleExit();
    void HandleMapView();

    IGuiManager& mGuiManager;
    const Icons& mIcons;
    const Font& mSpellFont;
    const Font& mGameFont;

    BAK::Layout mLayout;
    BAK::Layout mMapLayout;

    std::vector<Widget> mActiveSpells;
    Compass mCompass;
    std::vector<ClickButtonImage> mButtons;
    std::vector<ClickButtonImage> mMapButtons;
    std::vector<ClickButtonImage> mCharacters;
    unsigned mFollowRoadOffImage;
    unsigned mFollowRoadOnImage;

    bool mCanSaveBookmark{false};
    bool mFollowRoadButtonVisible{false};
    bool mZoomOutVisible{false};
    bool mZoomInVisible{false};
    bool mShowingBookmarkDialog{false};
    bool mNeedRefresh{false};
    bool mIsInMapView{false};
    ClickButton mBookmarkPopup;

    const Logging::Logger& mLogger;
};

}
