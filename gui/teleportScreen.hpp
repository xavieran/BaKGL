#pragma once

#include "bak/layout.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/clickButton.hpp"
#include "gui/teleportDest.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <optional>

namespace Logging {
class Logger;
}

namespace BAK {
class GameState;
struct ShopStats;
}
namespace Gui {

class IGuiManager;
class Backgrounds;
class Font;
class Icons;
class TeleportScreen : public Widget, public IDialogScene
{
    enum class State
    {
        Idle,
        Teleported,
        Cancelled
    };

public:
    static constexpr auto sLayoutFile = "REQ_TELE.DAT";
    static constexpr auto sCancelWidget = 12;

    TeleportScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);
    
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;
	bool OnMouseEvent(const MouseEvent& me) override;

    void SetSourceTemple(unsigned sourceTemple, BAK::ShopStats* temple);
private:
    void HandleTempleClicked(unsigned templeNumber);
    void HandleTempleHighlighted(unsigned templeNumber, bool selected);
    std::string MakeTempleString(const std::string& prefix, unsigned templeNumber);
    std::string MakeCostString(unsigned templeNumber);
    void AddChildren();
    BAK::Royals CalculateCost(unsigned templeNumber);

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;

    State mState;
    unsigned mSource;
    BAK::ShopStats* mTemple;
    std::optional<unsigned> mHighlightedDest;
    std::optional<unsigned> mChosenDest;
    bool mNeedRefresh;

    Widget mTeleportWord;
    TextBox mTeleportFromText;
    Widget mTeleportFrom;
    TextBox mTeleportToText;
    Widget mTeleportTo;
    TextBox mCostText;
    Widget mMapSnippet;
    Widget mMap;
    ClickButton mCancelButton;

    std::vector<TeleportDest> mTeleportDests;

    const Logging::Logger& mLogger;
};

}
