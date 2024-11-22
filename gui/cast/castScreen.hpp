#pragma once

#include "bak/layout.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/cast/symbol.hpp"
#include "gui/cast/powerRing.hpp"

#include "gui/core/line.hpp"

#include "gui/clickButton.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {
class IGuiManager;
class Backgrounds;
class Icons;
class Font;
}

namespace BAK {
class GameState;
}

namespace Gui::Cast {

class CastScreen : public Widget, public NullDialogScene
{
    enum class State
    {
        Idle,
        SpellSelected,
    };
    static constexpr auto sLayoutFile = "REQ_CAST.DAT";
    static constexpr auto sCombatLayoutFile = "SPELL.DAT";
    static constexpr auto sCastPanel = "CAST.SCX";
    static constexpr auto sScreen = "CFRAME.SCX";

    static constexpr auto sSymbol6 = 0;
    static constexpr auto sSymbol5 = 1;
    static constexpr auto sSymbol2 = 2;
    static constexpr auto sSymbol4 = 3;
    static constexpr auto sExit = 6;

    static constexpr auto sCharacterWidgetBegin = 7;

    static constexpr auto sSymbolTransitionTimeSeconds = .5;

public:
    CastScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        const Font& spellFont,
        BAK::GameState& gameState);

    bool OnMouseEvent(const MouseEvent& event) override;
    void BeginCast(bool inCombat);

private:
    void PrepareLayout();
    void SetActiveCharacter(BAK::ActiveCharIndex character);
    void HandleSpellClicked(BAK::SpellIndex spellIndex);
    void HandleSpellPower(unsigned power);
    void CastSpell(unsigned power);
    void HandleSpellHighlighted(BAK::SpellIndex spellIndex, bool selected);
    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override;
    void Exit();
    void HandleButton(unsigned i);
    void ChangeSymbol(unsigned newSymbol);
    void AddChildren();

    BAK::GameState& mGameState;

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::Layout mLayout;
    BAK::Layout mCombatLayout;

    State mState;
    bool mInCombat;
    std::optional<BAK::SpellIndex> mSelectedSpell;
    BAK::ActiveCharIndex mSelectedCharacter;

    Symbol mSymbol;

    std::vector<Line> mLines;
    std::vector<ClickButtonImage> mButtons;
    TextBox mSpellDesc;
    PowerRing mPowerRing;

    const Logging::Logger& mLogger;
};

}
