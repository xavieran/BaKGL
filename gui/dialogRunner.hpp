#pragma once

#include "bak/dialog.hpp"
#include "bak/types.hpp"

#include "com/assert.hpp"

#include "gui/choiceScreen.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/core/widget.hpp"

#include <stack>

namespace BAK {
class GameState;
class DialogSnippet;
}

namespace Gui {

class Actors;
class Backgrounds;
class IDialogScene;
class Font;
class ScreenStack;

struct DialogState;

class DialogRunner : public Widget
{
public:
    using FinishCallback = std::function<
        void(std::optional<BAK::ChoiceIndex>)>;

    DialogRunner(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState,
        ScreenStack& screenStack,
        FinishCallback&& finished);

    ~DialogRunner();

    const std::optional<BAK::ChoiceIndex>& GetLastChoice() const;
    std::optional<BAK::TeleportIndex> GetAndResetPendingTeleport();

    void SetDialogScene(IDialogScene* dialogScene);
    void SetInWorldView(bool);

    void BeginDialog(
        BAK::Target target,
        bool isTooltip);

    bool OnMouseEvent(const MouseEvent& event) override;

private:
    bool IsActive();
    
    bool LeftMousePressed(glm::vec2);
    bool MouseMoved(glm::vec2 pos);

    void EvaluateSnippetActions();
    void DisplaySnippet();

    const BAK::DialogSnippet& GetSnippet() const;
    std::optional<BAK::Target> GetNextTarget();

    void MakeChoice(BAK::ChoiceIndex choice);
    void ShowDialogChoices();
    void ShowQueryChoices();

    void ContinueDialogFromStack();
    bool RunDialog();
    void CompleteDialog();
    BAK::Target GetAndPopTargetStack();
    
    ScreenStack& mScreenStack;
    std::unique_ptr<DialogState> mDialogState;
    ChoiceScreen mChoices;
    BAK::Keywords mKeywords;
    BAK::GameState& mGameState;
    glm::vec2 mCenter{};

    const Font& mFont;
    const Actors& mActors;

    std::optional<BAK::Target> mCurrentTarget{};
    std::optional<BAK::ChoiceIndex> mLastChoice{};
    std::optional<BAK::TeleportIndex> mPendingZoneTeleport{};
    std::stack<BAK::Target> mTargetStack{};
    bool mStartedMusic{};
    bool mInWorldView{};

    std::string mRemainingText{};
    glm::vec2 mTextDims{};
    DialogDisplay mDialogDisplay;
    FinishCallback mFinished;
    IDialogScene* mDialogScene;

    const Logging::Logger& mLogger;
};

}
