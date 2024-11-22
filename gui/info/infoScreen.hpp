#pragma once

#include "bak/types.hpp"
#include "bak/dialogTarget.hpp"
#include "bak/layout.hpp"

#include "gui/IDialogScene.hpp"

#include "gui/info/portrait.hpp"
#include "gui/info/ratings.hpp"
#include "gui/info/skills.hpp"
#include "gui/info/spells.hpp"

#include "gui/clickButton.hpp"
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

class InfoScreen : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_INFO.DAT";
    static constexpr auto sSkillRightClickDialog = BAK::KeyTarget{0x143};

    static constexpr auto sPortraitWidget = 0;
    static constexpr auto sExitWidget = 1;
    static constexpr auto sSpellsWidget = 2;

    InfoScreen(
        IGuiManager& guiManager,
        const Actors& actors,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState);
    
    void SetSelectedCharacter(BAK::ActiveCharIndex character);

    void UpdateCharacter();
    void AdvanceCharacter();

private:
    void ToggleSkill(BAK::SkillType skill);
    void AddChildren();

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;
    BAK::ActiveCharIndex mSelectedCharacter;

    BAK::Layout mLayout;

    ClickButton mExitButton;
    ClickButton mSpellsButton;
    Portrait mPortrait;
    Ratings mRatings;
    Skills mSkills;
    SpellsScreen mSpells;
};

}
