#pragma once

#include "bak/dialog.hpp"

#include "gui/IDialogDisplay.hpp"
#include "gui/button.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <string>
#include <string_view>

namespace BAK {
class GameState;
}

namespace Gui {

class IDialogScene;

class Actors;
class Backgrounds;
class Font;

enum class DialogFrame
{
    Fullscreen = 0,
    ActionArea = 1,
    ActionAreaInventory = 2,
    LowerArea = 3,
    Popup = 4
};

class DialogDisplay : public Widget, IDialogDisplay
{
public:
    DialogDisplay(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState);
    
    void ShowWorldViewPane(bool isInWorldView);
    void DisplayPlayer(IDialogScene& dialogScene, unsigned act);

    std::pair<glm::vec2, std::string> DisplaySnippet(
        IDialogScene& dialogScene,
        const BAK::DialogSnippet& snippet,
        std::string_view remainingText,
        bool isInWorldView,
        glm::vec2 mousePos);

    void ShowFlavourText(BAK::Target target);
    void Clear();
private:
    void AddLabel(std::string_view text);

    std::pair<glm::vec2, std::string_view> SetText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredX,
        bool centeredY,
        bool isBold,
        bool isInWorldView);

    void SetActor(
        unsigned actor,
        IDialogScene& dialogScene,
        bool showName);

private:
    BAK::Keywords mKeywords;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;
    const Font& mFont;
    const Actors& mActors;
    Label mLabel;
    Widget mActor;

    Widget mFullscreenFrame;
    Widget mWorldViewFrame;
    Widget mActionAreaFrame;
    Widget mActionAreaBackground;
    Widget mLowerFrame;

    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;
    Button mPopup;
    TextBox mPopupText;

    const Logging::Logger& mLogger;
};

}
