#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"
#include "bak/types.hpp"

#include "com/algorithm.hpp"
#include "com/assert.hpp"
#include "com/random.hpp"
#include "com/visit.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/choiceScreen.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/frame.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <regex>
#include <variant>

namespace Gui {

class DialogRunner : public Widget
{
public:
    using FinishCallback = std::function<void(std::optional<BAK::ChoiceIndex>)>;

    DialogRunner(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState,
        ScreenStack& screenStack,
        FinishCallback&& finished)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{0},
            true
        },
        mScreenStack{screenStack},
        mDialogState{false, false, glm::vec2{0}, glm::vec2{0}},
        mChoices{
            pos,
            dims,
            fr,
            screenStack,
            [this](auto choice){
                MakeChoice(choice);
            }
        },
        mDialogStore{},
        mKeywords{},
        mGameState{gameState},
        mCenter{160, 112},
        mFont{fr},
        mActors{actors},
        mCurrentTarget{},
        mCurrentDialog{},
        mLastChoice{},
        mPendingZoneTeleport{},
        mRemainingText{""},
        mDialogDisplay{pos, dims, actors, bgs, fr, gameState},
        mFinished{finished},
        mDialogScene{nullptr},
        mLogger{Logging::LogState::GetLogger("Gui::DialogRunner")}
    {
        AddChildBack(&mDialogDisplay);
        ASSERT(mFinished);
    }

    ~DialogRunner()
    {
        mLogger.Debug() << "Destroyed: " << this << "\n";
    }

    const std::optional<BAK::ChoiceIndex>& GetLastChoice() const { return mLastChoice; }

    std::optional<BAK::TeleportIndex> GetAndResetPendingTeleport()
    {
        auto tmp = mPendingZoneTeleport;
        mPendingZoneTeleport.reset();
        return tmp;
    }


    void SetDialogScene(IDialogScene* dialogScene)
    {
        mDialogScene = dialogScene;
    }

    bool Active()
    {
        return mDialogState.mDialogActive 
            || mDialogState.mTooltipActive;
    }

    // if pos is set then exit the dialog as soon as the 
    // mouse moves away from pos (as in tooltips)
    void BeginDialog(
        BAK::Target target,
        bool isTooltip)
    {
        mLastChoice.reset();
        ASSERT(!mDialogState.mDialogActive);
        ASSERT(!mDialogState.mTooltipActive);
        if (isTooltip)
            mDialogState.ActivateTooltip();
        else
            mDialogState.ActivateDialog();

        mCurrentTarget = target;
        Logging::LogDebug("Gui::DialogRunner")
            << "BeginDialog" << target << " snip: " << mCurrentDialog << "\n";
        RunDialog(true);
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        return std::visit(overloaded{
            [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
            [this](const MouseMove& p){ return MouseMoved(p.mValue); },
            [](const auto& p){ return false; }
            },
            event);
    }

    bool LeftMousePressed(glm::vec2)
    {
        RunDialog();
        return true;
    }

    bool MouseMoved(glm::vec2 pos)
    {
        mDialogState.MouseMoved(pos);
        mDialogState.DeactivateTooltip(
            [this](){
                mDialogScene = nullptr;
                std::invoke(mFinished, GetLastChoice());
            });
        return false;
    }


    void EvaluateSnippetActions()
    {
        for (const auto& action : mCurrentDialog->mActions)
        {
            std::visit(overloaded{
                [&](const BAK::PushNextDialog& push){
                    mLogger.Debug() << "Pushing: " << push.mTarget << "\n";
                    mTargetStack.push(push.mTarget);
                },
                [&](const BAK::Teleport& teleport){
                    mLogger.Debug() << "Teleporting to zoneIndex: " << teleport.mIndex << "\n";
                    mPendingZoneTeleport = teleport.mIndex;
                },
                [&](const auto& a){
                    mGameState.EvaluateAction(BAK::DialogAction{action});
                }},
                action);
        }
    }

    void UpdateSnippet()
    {
        std::optional<std::string_view>  remaining{};
        if (!mRemainingText.empty())
            remaining = std::string_view{mRemainingText};
        auto nullDialog = NullDialogScene{};
        mRemainingText = mDialogDisplay.DisplaySnippet(
            mDialogScene ? *mDialogScene : nullDialog,
            *mCurrentDialog,
            remaining);
    }

    BAK::Target ProgressDialog()
    {
        if (mCurrentDialog && (mCurrentDialog->mDisplayStyle3 == 0x8))
        {
            const auto choice = GetRandomNumber(0, mCurrentDialog->GetChoices().size() - 1);
            return mCurrentDialog->GetChoices()[choice].mTarget;
        }
        else if (mCurrentDialog && mCurrentDialog->GetChoices().size() >= 1)
        {
            for (const auto& c : mCurrentDialog->GetChoices())
            {
                if (mGameState.EvaluateDialogChoice(c.mChoice))
                    return c.mTarget;
            }

            return mCurrentDialog->GetChoices().back().mTarget;
        }
        else
        {
            return *mCurrentTarget;
        }
    }

    void MakeChoice(BAK::ChoiceIndex choice)
    {
        mLogger.Debug() << "Made choice: " << choice << "\n";
        mScreenStack.PopScreen();

        const auto& choices = mCurrentDialog->GetChoices();
        const auto it = std::find_if(choices.begin(), choices.end(),
            [choice](const auto& c){
                if (std::holds_alternative<BAK::ConversationChoice>(c.mChoice))
                    return std::get<BAK::ConversationChoice>(c.mChoice).mEventPointer == choice.mValue;
                else if (std::holds_alternative<BAK::QueryChoice>(c.mChoice))
                    return std::get<BAK::QueryChoice>(c.mChoice).mQueryIndex == choice.mValue;
                else
                    return false;
            });

        mLastChoice = choice;

        if (it == choices.end())
        {
            // usual "Yes/No" dialogs
            if (choices.size() == 0 || choices.size() == 1)
            {
                mLogger.Info() << "Yes|No dialog choice. Player chose: " << mLastChoice << "\n";

                mCurrentDialog = std::optional<BAK::DialogSnippet>{};
                mRemainingText = std::string{};
                RunDialog();
            }
            else
            {
                // Break out of the question loop
                mTargetStack.pop();
                mCurrentTarget = mTargetStack.top();
                mCurrentDialog = mDialogStore.GetSnippet(*mCurrentTarget);
                mTargetStack.pop();
                UpdateSnippet();
            }
        }
        else
        {
            evaluate_if<BAK::ConversationChoice>(
                it->mChoice,
                [&](const auto& c){
                    mGameState.MarkDiscussed(c);
                });

            mCurrentTarget = it->mTarget;
            mCurrentDialog = mDialogStore.GetSnippet(*mCurrentTarget);
            EvaluateSnippetActions();
            mCurrentDialog = std::optional<BAK::DialogSnippet>{};
            mRemainingText = std::string{};
            // blergh this and the above are really unpleasant this
            // flow could be improved...
            RunDialog(true);
        }
    }

    void ShowDialogChoices()
    {
        mLogger.Debug() << "DialogChoices: " << mCurrentDialog << "\n";
        mChoices.SetPosition(glm::vec2{15, 125});
        mChoices.SetDimensions(glm::vec2{295, 66});

        auto choices = std::vector<std::pair<BAK::ChoiceIndex, std::string>>{};
        unsigned i = 0;
        for (const auto& c : mCurrentDialog->GetChoices())
        {
            if (std::holds_alternative<BAK::ConversationChoice>(c.mChoice))
            {
                const auto choice = std::get<BAK::ConversationChoice>(c.mChoice);
                if (mGameState.GetEventStateBool(choice.mEventPointer)
                    && !mGameState.CheckInhibited(choice))
                {
                    const auto fontStyle = mGameState.CheckDiscussed(choice)
                        ? '\xf4' // unbold
                        : '#';
                    choices.emplace_back(
                        std::make_pair(
                            BAK::ChoiceIndex{choice.mEventPointer},
                            fontStyle + std::string{
                                mKeywords.GetDialogChoice(choice.mEventPointer)}));

                }
            }
        }
        choices.emplace_back(std::make_pair(-1, "Goodbye"));
        constexpr auto buttonSize = glm::vec2{68, 14};
        mChoices.StartChoices(choices, buttonSize);

        mDialogDisplay.Clear();
        if (mDialogScene)
            mDialogDisplay.DisplayPlayer(*mDialogScene, mCurrentDialog->mActor);
        mScreenStack.PushScreen(&mChoices);
    }

    void ShowQueryChoices()
    {
        mLogger.Debug() << "QueryChoices: " << mCurrentDialog << "\n";
        mChoices.SetPosition(glm::vec2{15, 125});
        mChoices.SetDimensions(glm::vec2{285, 66});

        auto choices = std::vector<std::pair<BAK::ChoiceIndex, std::string>>{};
        unsigned i = 0;
        for (const auto& c : mCurrentDialog->GetChoices())
        {
            if (std::holds_alternative<BAK::QueryChoice>(c.mChoice))
            {
                const auto index = std::get<BAK::QueryChoice>(c.mChoice).mQueryIndex;
                mLogger.Debug() << " Choice Index: " << index << "\n";
                choices.emplace_back(
                    std::make_pair(
                        BAK::ChoiceIndex{index},
                        "#" + std::string{
                            mKeywords.GetQueryChoice(index)}));
            }
            else
            {
                throw std::runtime_error("Non-query choice in query choice display");
            }
        }

        if (choices.size() == 0)
        {
            const auto index = BAK::Keywords::sYesIndex; // Yes
            choices.emplace_back(
                std::make_pair(
                    BAK::ChoiceIndex{index},
                    "#" + std::string{mKeywords.GetQueryChoice(index)}));
        }

        if (choices.size() == 1)
        {
            const auto index = BAK::Keywords::sNoIndex; // No
            choices.emplace_back(
                std::make_pair(
                    BAK::ChoiceIndex{index},
                    "#" + std::string{mKeywords.GetQueryChoice(index)}));
        }
        

        constexpr auto buttonSize = glm::vec2{32, 14};
        mChoices.StartChoices(choices, buttonSize);

        mScreenStack.PushScreen(&mChoices);
    }

    bool RunDialog(bool first=false)
    {
        bool progressing = true;
        mLogger.Debug() << "RunDialog first: [" << first << "] rem: "
            << mRemainingText << " CurTarg: " << mCurrentTarget 
            << " CurSnip: " << mCurrentDialog << "\n";

        if (!mRemainingText.empty())
        {
            UpdateSnippet();
            return true;
        }

        constexpr std::string_view empty = "";
        unsigned iters = 0;
        do
        {
            BAK::Target current;
            if (!mCurrentDialog && mCurrentTarget)
            {
                current = *mCurrentTarget;
            }
            else
            {
                current = ProgressDialog();
            }

            auto currentDialog = mDialogStore.GetSnippet(current);
            mLogger.Debug() << "Progressed through: " << current 
                << "(" << mCurrentTarget << ") " << currentDialog << std::endl;
            if (current == *mCurrentTarget && !first)
            {
                if (!mTargetStack.empty())
                {
                    current = mTargetStack.top();
                    currentDialog = mDialogStore.GetSnippet(current);
                    mTargetStack.pop();
                    mLogger.Debug() << "Evaluating stacked dialog\n";
                }
                else
                {
                    progressing = false;
                    mLogger.Debug() << "Finished dialog\n";
                    mDialogDisplay.Clear();
                    mDialogState.DeactivateDialog();
                    mDialogScene = nullptr;
                    std::invoke(mFinished, GetLastChoice());

                    return false;
                }
            }

            mCurrentTarget = current;
            mCurrentDialog = currentDialog;
            EvaluateSnippetActions();

            const auto text = mCurrentDialog->GetText();
            if (text != empty)
            {
                UpdateSnippet();
                if (mCurrentDialog->mDisplayStyle3 == 0x2)
                    //&& mCurrentDialog->GetChoices().size() > 0)
                    ShowQueryChoices();
                progressing = false;
            }
            else if (mCurrentDialog->mDisplayStyle3 == 0x4)
            {
                ShowDialogChoices();
                progressing = false;
            }

        // FIXME: Fix the infinite loop sometimes...
        } while (progressing && (iters++ < 20));

        return true;
    }

private:
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
    BAK::DialogStore mDialogStore;
    BAK::Keywords mKeywords;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;

    const Font& mFont;
    const Actors& mActors;

    std::optional<BAK::Target> mCurrentTarget;
    std::optional<BAK::DialogSnippet> mCurrentDialog;
    std::optional<BAK::ChoiceIndex> mLastChoice;
    std::optional<BAK::TeleportIndex> mPendingZoneTeleport;
    std::stack<BAK::Target> mTargetStack;

    std::string mRemainingText;
    DialogDisplay mDialogDisplay;
    FinishCallback mFinished;
    IDialogScene* mDialogScene;

    const Logging::Logger& mLogger;
};

}
