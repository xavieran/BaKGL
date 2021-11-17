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
        mKeywords{},
        mGameState{gameState},
        mCenter{160, 112},
        mFont{fr},
        mActors{actors},
        mCurrentTarget{},
        mLastChoice{},
        mPendingZoneTeleport{},
        mRemainingText{""},
        mTextDims{0},
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

    void BeginDialog(
        BAK::Target target,
        bool isTooltip)
    {
        mLastChoice.reset();

        ASSERT(!Active());

        if (isTooltip)
            mDialogState.ActivateTooltip();
        else
            mDialogState.ActivateDialog();

        mTargetStack.push(target);
        mLogger.Debug() << "BeginDialog" << target << "\n";
        ContinueDialogFromStack();
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

    void CompleteDialog()
    {
        mLogger.Debug() << "Finished dialog\n";
        mDialogDisplay.Clear();
        mDialogState.DeactivateDialog();
        mDialogScene = nullptr;
        mCurrentTarget.reset();
        mRemainingText.clear();
        std::invoke(mFinished, GetLastChoice());
        // reset last choice...?
    }

    bool LeftMousePressed(glm::vec2)
    {
        if (mDialogState.mTooltipActive)
        {
            mDialogState.ForceDeactivateTooltip(
            [this](){
                mDialogScene = nullptr;
                std::invoke(mFinished, GetLastChoice());
            });
        }
        else
        {
            RunDialog();
        }

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
        mLogger.Debug() << "Evaluating actions for " << mCurrentTarget << "\n";
        for (const auto& action : GetSnippet().mActions)
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

    void DisplaySnippet()
    {
        std::string text{};
        if (!mRemainingText.empty())
            text = mRemainingText;
        else
        {
            text = GetSnippet().GetText();
            // Bit hacky: Add an extra line to ensure enough room to
            // display the query choice
            if (GetSnippet().IsQueryChoice())
                text += "\n";
        }
        auto nullDialog = NullDialogScene{};

        const auto [textDims, rem] = mDialogDisplay.DisplaySnippet(
            mDialogScene ? *mDialogScene : nullDialog,
            GetSnippet(),
            text);
        mTextDims = textDims;
        mRemainingText = rem;
    }

    const BAK::DialogSnippet& GetSnippet() const
    {
        ASSERT(mCurrentTarget);
        return BAK::GetDialogStore().GetSnippet(*mCurrentTarget);
    }

    std::optional<BAK::Target> GetNextTarget()
    {
        if (!mCurrentTarget)
        { 
            if (!mTargetStack.empty())
                return GetAndPopTargetStack();
            else
                return std::optional<BAK::Target>{};
        }

        const auto& snip = GetSnippet();
        if (snip.IsRandomChoice())
        {
            ASSERT(snip.GetChoices().size() > 0);
            const auto choice = GetRandomNumber(0, snip.GetChoices().size() - 1);
            return snip.GetChoices()[choice].mTarget;
        }
        else if (snip.GetChoices().size() >= 1)
        {
            for (const auto& c : snip.GetChoices())
            {
                if (mGameState.EvaluateDialogChoice(c.mChoice))
                    return c.mTarget;
            }
        }

        if (!mTargetStack.empty())
        {
            return GetAndPopTargetStack();
        }

        return std::optional<BAK::Target>{};
    }

    void MakeChoice(BAK::ChoiceIndex choice)
    {
        mLogger.Debug() << "Made choice: " << choice << "\n";
        mScreenStack.PopScreen();

        const auto& choices = GetSnippet().GetChoices();
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
            }
            else
            {
                // Break out of the question loop
                ASSERT(!mTargetStack.empty());
                mTargetStack.pop();
            }
        }
        else
        {
            evaluate_if<BAK::ConversationChoice>(
                it->mChoice,
                [&](const auto& c){
                    mGameState.MarkDiscussed(c);
                });

            mTargetStack.push(it->mTarget);
        }

        ContinueDialogFromStack();
    }

    void ShowDialogChoices()
    {
        mLogger.Debug() << "DialogChoices: " << GetSnippet() << "\n";
        mChoices.SetPosition(glm::vec2{15, 125});
        mChoices.SetDimensions(glm::vec2{295, 66});

        auto choices = std::vector<std::pair<BAK::ChoiceIndex, std::string>>{};
        unsigned i = 0;
        for (const auto& c : GetSnippet().GetChoices())
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
            mDialogDisplay.DisplayPlayer(*mDialogScene, GetSnippet().mActor);
        mScreenStack.PushScreen(&mChoices);
    }

    void ShowQueryChoices()
    {
        mLogger.Debug() << "QueryChoices: " << GetSnippet() << "\n";
        mLogger.Debug() << "Last text dims:" << mTextDims << "\n";
        mChoices.SetPosition(glm::vec2{100, mTextDims.y});
        mChoices.SetDimensions(glm::vec2{285, 66});

        auto choices = std::vector<std::pair<BAK::ChoiceIndex, std::string>>{};
        unsigned i = 0;
        for (const auto& c : GetSnippet().GetChoices())
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

    void ContinueDialogFromStack()
    {
        mRemainingText.clear();
        mCurrentTarget.reset();
        RunDialog();
    }

    bool RunDialog()
    {
        mLogger.Debug() << "RunDialog rem: ["
            << mRemainingText << "] CurTarg: " << mCurrentTarget <<"\n";

        if (!mRemainingText.empty())
        {
            DisplaySnippet();
            if (mRemainingText.empty() 
                && GetSnippet().IsQueryChoice())
                ShowQueryChoices();

            return true;
        }

        if (mCurrentTarget && GetSnippet().IsQueryChoice())
        {
            ShowQueryChoices();
            return true;
        }

        do
        {
            const auto nextTarget = GetNextTarget();
            mLogger.Debug() << "Curr: " << mCurrentTarget 
                << " Next Target: " << nextTarget << "\n";
            if (!nextTarget)
            {
                CompleteDialog();
                return false;
            }
            else
            {
                mCurrentTarget = *nextTarget;
                EvaluateSnippetActions();
                mLogger.Debug() << "Progressing through: " << GetSnippet() << "\n";
            }
        } while (!mCurrentTarget
            || !GetSnippet().IsDisplayable());

        if (GetSnippet().IsDialogChoice())
            ShowDialogChoices();
        else
            DisplaySnippet();

        if (GetSnippet().IsQueryChoice() && mRemainingText.empty())
            ShowQueryChoices();

        return true;
    }

    BAK::Target GetAndPopTargetStack()
    {
        ASSERT(!mTargetStack.empty());
        const auto target = mTargetStack.top();
        mTargetStack.pop();
        return target;
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

    std::string mRemainingText;
    glm::vec2 mTextDims;
    DialogDisplay mDialogDisplay;
    FinishCallback mFinished;
    IDialogScene* mDialogScene;

    const Logging::Logger& mLogger;
};

}
