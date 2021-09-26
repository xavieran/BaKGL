#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/algorithm.hpp"
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
    using FinishCallback = std::function<void()>;

    DialogRunner(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState,
        ScreenStack& screenStack,
        IDialogScene& dialogScene)
    :
        DialogRunner{
            pos,
            dims,
            actors,
            bgs,
            fr,
            gameState,
            screenStack,
            dialogScene,
            []{}}
    {}

    DialogRunner(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState,
        ScreenStack& screenStack,
        IDialogScene& dialogScene,
        FinishCallback&& finished)
    :
        Widget{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            //Color::debug,
            glm::vec4{0},
            pos,
            dims,
            true
        },
        mScreenStack{screenStack},
        mDialogScene{dialogScene},
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
        mRemainingText{""},
        mDialogDisplay{pos, dims, actors, bgs, fr, gameState},
        mFinished{std::move(finished)},
        mLogger{Logging::LogState::GetLogger("Gui::DialogRunner")}
    {
        AddChildBack(&mDialogDisplay);
        assert(mFinished);
    }

    bool Active()
    {
        return mDialogState.mDialogActive 
            || mDialogState.mTooltipActive;
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
                [&](const BAK::SetFlag& set)
                {
                    mLogger.Debug() << "Setting flag of event: " << BAK::DialogAction{set} << "\n";
                    mGameState.SetEventState(set);
                },
                [&](const auto& a){
                    mLogger.Debug() << "Doing nothing for: " << a << "\n";
                }},
                action);
        }
    }

    void UpdateSnippet()
    {
        std::optional<std::string_view>  remaining{};
        if (!mRemainingText.empty())
            remaining = std::string_view{mRemainingText};

        mRemainingText = mDialogDisplay.DisplaySnippet(
            mDialogScene,
            *mCurrentDialog,
            remaining);

        mLogger.Debug() << "Snippet: " << mCurrentDialog << "\n";
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

    // if pos is set then exit the dialog as soon as the 
    // mouse moves away from pos (as in tooltips)
    void BeginDialog(
        BAK::Target target,
        bool isTooltip)
    {
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
                std::invoke(mFinished);
            });
        return false;
    }

    void MakeChoice(ChoiceScreen::ChoiceIndex choice)
    {
        mLogger.Debug() << "Made choice: " << choice << "\n";
        mScreenStack.PopScreen();

        const auto& choices = mCurrentDialog->GetChoices();
        const auto it = std::find_if(choices.begin(), choices.end(),
            [choice](const auto& c){
                if (std::holds_alternative<BAK::ConversationChoice>(c.mChoice))
                    return std::get<BAK::ConversationChoice>(c.mChoice).mEventPointer == choice;
                else if (std::holds_alternative<BAK::QueryChoice>(c.mChoice))
                    return std::get<BAK::QueryChoice>(c.mChoice).mQueryIndex == choice;
                else
                    return false;
            });
        if (it == choices.end())
        {
            // Break out of the question loop
            mTargetStack.pop();
            mCurrentTarget = mTargetStack.top();
            mCurrentDialog = mDialogStore.GetSnippet(*mCurrentTarget);
            mTargetStack.pop();
            UpdateSnippet();
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
            // blergh this and the above are really unpleasant this
            // flow could be improved...
            EvaluateSnippetActions();
            RunDialog();
        }
    }

    void ShowDialogChoices()
    {
        mLogger.Debug() << "DialogChoices: " << mCurrentDialog << "\n";
        mChoices.SetPosition(glm::vec2{15, 125});
        mChoices.SetDimensions(glm::vec2{295, 66});

        auto choices = std::vector<std::pair<unsigned, std::string>>{};
        unsigned i = 0;
        for (const auto& c : mCurrentDialog->GetChoices())
        {
            if (std::holds_alternative<BAK::ConversationChoice>(c.mChoice))
            {
                const auto choice = std::get<BAK::ConversationChoice>(c.mChoice);
                if (mGameState.GetEventState(choice.mEventPointer)
                    && !mGameState.CheckInhibited(choice))
                {
                    const auto fontStyle = mGameState.CheckDiscussed(choice)
                        ? '\xf4' // unbold
                        : '#';
                    choices.emplace_back(
                        std::make_pair(
                            choice.mEventPointer,
                            fontStyle + std::string{
                                mKeywords.GetDialogChoice(choice.mEventPointer)}));

                }
            }
        }
        choices.emplace_back(std::make_pair(-1, "Goodbye"));
        constexpr auto buttonSize = glm::vec2{68, 14};
        mChoices.StartChoices(choices, buttonSize);

        mDialogDisplay.Clear();
        mDialogDisplay.DisplayPlayer(mDialogScene);
        mScreenStack.PushScreen(&mChoices);
    }

    void ShowQueryChoices()
    {
        mLogger.Debug() << "QueryChoices: " << mCurrentDialog << "\n";
        mChoices.SetPosition(glm::vec2{15, 125});
        mChoices.SetDimensions(glm::vec2{285, 66});

        auto choices = std::vector<std::pair<unsigned, std::string>>{};
        unsigned i = 0;
        for (const auto& c : mCurrentDialog->GetChoices())
        {
            if (std::holds_alternative<BAK::QueryChoice>(c.mChoice))
            {
                const auto index = std::get<BAK::QueryChoice>(c.mChoice).mQueryIndex;
                mLogger.Debug() << " Choice Index: " << index << "\n";
                choices.emplace_back(
                    std::make_pair(
                        index,
                        "#" + std::string{
                            mKeywords.GetQueryChoice(index)}));
            }
            else
            {
                throw std::runtime_error("Non-query choice in query choice display");
            }
        }
        constexpr auto buttonSize = glm::vec2{32, 14};
        mChoices.StartChoices(choices, buttonSize);

        //mDialogDisplay.Clear();
        //mDialogDisplay.DisplayPlayer(mDialogScene);
        mScreenStack.PushScreen(&mChoices);
    }

    // Call this when the player moves the dialog forward
    bool RunDialog(bool first=false)
    {
        bool progressing = true;

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

                    std::invoke(mFinished);

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
    IDialogScene& mDialogScene;
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
    std::stack<BAK::Target> mTargetStack;
    std::string mRemainingText;
    DialogDisplay mDialogDisplay;
    FinishCallback mFinished;

    const Logging::Logger& mLogger;
};

}
