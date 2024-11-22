#pragma once

#include "bak/camp.hpp"
#include "bak/layout.hpp"
#include "bak/money.hpp"
#include "bak/worldClock.hpp"

#include "gui/IDialogScene.hpp"

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"

#include "gui/camp/clock.hpp"
#include "gui/clickButton.hpp"

#include <glm/glm.hpp>

#include <utility>

namespace BAK {
class GameState;
struct ShopStats;
}

namespace Gui {
class IGuiManager;
class Backgrounds;
class Icons;
class Font;
class TickAnimator;
}

namespace Gui::Camp {

class CampScreen : public Widget, public NullDialogScene
{
    enum class State
    {
        Idle,
        Camping,
        CampingTilHealed
    };
    static constexpr auto sLayoutFile = "REQ_CAMP.DAT";
    static constexpr auto sScreen = "ENCAMP.SCX";

    static constexpr auto sCampUntilHealed = 0;
    static constexpr auto sStop = 1;
    static constexpr auto sExit = 2;

public:
    CampScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);

    bool OnMouseEvent(const MouseEvent& event) override;
    void BeginCamp(bool isInn, BAK::ShopStats* shopStats);
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;

private:
    BAK::Royals GetInnCost();
    void ShowInnDialog(bool haveSlept);
    void SetText();
    bool AnyCharacterCanHeal();
    void HandleDotClicked(unsigned i);
    unsigned GetHour();
    void StartCamping(std::optional<unsigned> hourTil);
    void HandleTick();
    void FinishedTicking();
    void HandleButton(unsigned button);
    void Exit();
    void AddChildren();

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;
    BAK::CampData mCampData;

    Widget mFrame;
    TextBox mNamesColumn;
    TextBox mHealthColumn;
    TextBox mRationsColumn;
    TextBox mPartyGold;

    std::vector<ClickButtonImage> mButtons;
    using ClockTick = Highlightable<
        Clickable<
            detail::CampDest,
            LeftMousePress,
            std::function<void()>>,
        true>;
    std::vector<ClockTick> mDots;

    bool mIsInInn;
    State mState;
    std::optional<unsigned> mTargetHour;
    BAK::Time mTimeBeganCamping;
    TickAnimator* mTimeElapser;
    BAK::ShopStats* mShopStats;

    const Logging::Logger& mLogger;
};

}
