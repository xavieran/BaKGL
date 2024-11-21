#pragma once

#include "bak/layout.hpp"
#include "bak/types.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/info/portrait.hpp"
#include "gui/info/ratings.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class GameState;
}

namespace Gui {

class IGuiManager;

class Actors;
class Backgrounds;
class Font;
class Icons;

class CureScreen : public Widget, public IDialogScene
{
public:
    static constexpr auto sLayoutFile = "REQ_HEAL.DAT";
    static constexpr auto sInfoLayoutFile = "REQ_INFO.DAT";
    static constexpr auto sScreen = "OPTIONS1.SCX";

    static constexpr auto sCurePlayer = 0;
    static constexpr auto sNextPlayer = 1;
    static constexpr auto sDone = 2;

    static constexpr auto sPortraitWidget = 0;

    CureScreen(
        IGuiManager& guiManager,
        const Actors& actors,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);
    
    void DisplayNPCBackground() override;
    void DisplayPlayerBackground() override;
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;

    void EnterScreen(unsigned templeNumber, unsigned cureFactor, std::function<void()>&& finished);
private:
    void CureCharacter();
    void AdvanceCharacter();
    void SetSelectedCharacter(BAK::ActiveCharIndex character);
    BAK::Royals CalculateCureCost();
    void UpdateCharacter();

    void AddChildren();

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;
    BAK::Layout mInfoLayout;

    BAK::ActiveCharIndex mSelectedCharacter;
    unsigned mTempleNumber;
    unsigned mCureFactor;
    std::function<void()> mFinished;
    BAK::Royals mCost;
    Portrait mPortrait;
    Ratings mRatings;
    TextBox mCureText;
    ClickButton mCureButton;
    ClickButton mNextPlayerButton;
    ClickButton mDoneButton;

    const Logging::Logger& mLogger;
};

}
