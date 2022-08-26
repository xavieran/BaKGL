#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/types.hpp"

#include "com/algorithm.hpp"
#include "com/assert.hpp"
#include "com/ostream.hpp"
#include "com/random.hpp"
#include "com/visit.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/choiceScreen.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <regex>
#include <stack>
#include <variant>

namespace Gui {

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

    const std::optional<BAK::ChoiceIndex>& GetLastChoice() const { return mLastChoice; }
    std::optional<BAK::TeleportIndex> GetAndResetPendingTeleport();

    void SetDialogScene(IDialogScene* dialogScene);

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
    
    struct DialogState
    {
        void ActivateDialog()
        {
            mDialogActive = true;
        }

        void MouseMoved(glm::vec2 pos)
        {
            mMousePos = pos;
        }

        void ActivateTooltip()
        {
            mTooltipPos = mMousePos;
            mTooltipActive = true;
        }

        void DeactivateDialog()
        {
            mDialogActive = false;
        }

        template <typename F>
        void ForceDeactivateTooltip(F&& f)
        {
            ASSERT(mTooltipActive);
            f();
            mTooltipActive = false;
        }

        template <typename F>
        void DeactivateTooltip(F&& f)
        {
            constexpr auto tooltipSensitivity = 15;
            if (mTooltipActive 
                && glm::distance(mMousePos, mTooltipPos) > tooltipSensitivity)
            {
                f();
                mTooltipActive = false;
            }
        }

        bool mDialogActive;
        bool mTooltipActive;
        glm::vec2 mMousePos;
        glm::vec2 mTooltipPos;
    };

    ScreenStack& mScreenStack;
    DialogState mDialogState;
    ChoiceScreen mChoices;
    BAK::Keywords mKeywords;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;

    const Font& mFont;
    const Actors& mActors;

    std::optional<BAK::Target> mCurrentTarget;
    std::optional<BAK::ChoiceIndex> mLastChoice;
    std::optional<BAK::TeleportIndex> mPendingZoneTeleport;
    std::stack<BAK::Target> mTargetStack;
    bool mStartedMusic;

    std::string mRemainingText;
    glm::vec2 mTextDims;
    DialogDisplay mDialogDisplay;
    FinishCallback mFinished;
    IDialogScene* mDialogScene;

    const Logging::Logger& mLogger;
};

}
