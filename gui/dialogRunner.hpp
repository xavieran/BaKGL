#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/frame.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <regex>

namespace Gui {

enum class DialogFrame
{
    Fullscreen = 0,
    ActionArea = 1,
    LowerArea = 2
};

class DialogRunner : public Widget
{
public:
    DialogRunner(
        glm::vec2 pos,
        glm::vec2 dims,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState)
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
        mDialogStore{},
        mGameState{gameState},
        mCenter{150, 0},
        mFont{fr},
        mLabel{
            mCenter,
            glm::vec2{320, 20},
            mFont,
            "#LABEL#"},
        mFullscreenFrame{
            Graphics::DrawMode::Sprite,
            bgs.GetSpriteSheet(),
            bgs.GetScreen("OPTIONS2.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            true
        },
        mActionAreaFrame{
            glm::vec2{15, 11},
            glm::vec2{289, 101},
            true
        },
        mActionAreaBackground{
            Graphics::DrawMode::Sprite,
            bgs.GetSpriteSheet(),
            bgs.GetScreen("DIALOG.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{-15, -11},
            glm::vec2{320, 200},
            true
        },
        mLowerFrame{
            glm::vec2{15, 120},
            glm::vec2{260, 100}
        },
        mFullscreenTextBox{
            glm::vec2{30, 30},
            glm::vec2{320 - 30*2, 135}},
        mActionAreaTextBox{
            glm::vec2{4, 4},
            glm::vec2{290, 103}},
        mLowerTextBox{
            glm::vec2{0, 0},
            glm::vec2{290, 70}},
        mLogger{Logging::LogState::GetLogger("Gui::DialogRunner")}
    {
        mFullscreenFrame.AddChildBack(&mFullscreenTextBox);

        mLowerFrame.AddChildBack(&mLowerTextBox);

        mActionAreaFrame.AddChildBack(&mActionAreaBackground);
        mActionAreaFrame.AddChildBack(&mActionAreaTextBox);
    }

    //const std::vector<Graphics::IGuiElement*>& GetChildren() const override
    //{
    //    return mActiveFrame;
    //}
    
    void AddText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredX,
        bool centeredY,
        bool isBold)
    {
        ClearChildren();
        switch (dialogFrame)
        {
        case DialogFrame::Fullscreen:
        {
            mFullscreenTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
            AddChildBack(&mFullscreenFrame);
        } break;
        case DialogFrame::ActionArea:
        {
            mActionAreaTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
            AddChildBack(&mActionAreaFrame);
        } break;
        case DialogFrame::LowerArea:
        {
            mLowerTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
            AddChildBack(&mLowerFrame);
        } break;
        }
    }

    void UpdateSnippet()
    {
        auto text = std::string{mCurrentDialog->GetText()};

        text = std::regex_replace(
            text,
            std::regex{"@4"},
            std::string{mGameState.GetPartyLeader()});

        text = std::regex_replace(
            text,
            std::regex{"@0"},
            std::string{mGameState.GetPartyFollower()});

        const auto ds1 = mCurrentDialog->mDisplayStyle;
        const auto ds3 = mCurrentDialog->mDisplayStyle3;
        const auto act = mCurrentDialog->mActor;

        const auto dialogFrame = std::invoke([ds1, ds3, act]{
            if (act != 0x0)
                return DialogFrame::LowerArea;

            if (ds3 == 0x02)
            {
                return DialogFrame::ActionArea;
            }

            if (ds1 == 0x02)
                return DialogFrame::ActionArea;
            else if (ds1 == 0x03
                || ds1 == 0x04)
                return DialogFrame::LowerArea;
            else
                return DialogFrame::Fullscreen;
        });
        
        const auto ds2 = mCurrentDialog->mDisplayStyle2;
        const bool verticallyCentered
            = ((ds2 & 0x10) == 0x10) || (ds2 == 0x3);
        const bool horizontallyCentered 
            = (ds2 & 0x4) == 0x4;
        const bool isBold
            = ds2 == 0x3;

        AddText(
            text,
            dialogFrame,
            horizontallyCentered,
            verticallyCentered,
            isBold);

        mLogger.Debug() << "Snippet: " << mCurrentDialog << "\n";
    }

    BAK::Target ProgressDialog()
    {
        auto current = BAK::Target{BAK::KeyTarget{0}};

        if (mCurrentDialog && mCurrentDialog->GetChoices().size() >= 1)
        {
            for (const auto& c : mCurrentDialog->GetChoices())
            {
                const auto choiceState 
                    = static_cast<BAK::ChoiceState>(c.mState);

                if (choiceState == BAK::ChoiceState::Chapter
                    && mGameState.GetChapter() == c.mChoice1)
                {
                    return c.mTarget;
                }
                else if (choiceState == BAK::ChoiceState::Money
                    && mGameState.GetMoney() > c.mChoice1)
                {
                    return c.mTarget;
                }
                else if (choiceState == BAK::ChoiceState::Time
                    && mGameState.GetTime() == c.mChoice1)
                {
                    return c.mTarget;
                }
                else if (c.mState == 0x0)
                {
                    return c.mTarget;
                }
            }

            return mCurrentDialog->GetChoices().back().mTarget;
        }
        else
        {
            return *mCurrentTarget;
        }
    }

    void BeginDialog(BAK::Target target)
    {
        mCurrentTarget = target;
        mCurrentDialog = mDialogStore.GetSnippet(target);
        Logging::LogDebug("Gui::DialogRunner")
            << "BeginDialog" << target << " snip: " << mCurrentDialog << "\n";
        RunDialog();
    }

    void RunDialog()
    {
        bool progressing = true;

        auto current = ProgressDialog();
        auto currentDialog = mDialogStore.GetSnippet(current);
        constexpr std::string_view empty = "";
        while (progressing)
        {
            const auto text = currentDialog.GetText();
            if (text != empty) 
            {
                mCurrentTarget = current;
                mCurrentDialog = currentDialog;
                UpdateSnippet();
                progressing = false;
            }
            else
            {
                current       = ProgressDialog();
                currentDialog = mDialogStore.GetSnippet(current);
                mLogger.Debug() << "Progressed through: " << current 
                    << " " << currentDialog << std::endl;
                if (current == *mCurrentTarget)
                    progressing = false;
                mCurrentTarget = current;
                mCurrentDialog = currentDialog;
            }
        }
    }

    void LeftMousePress(glm::vec2 click) override
    {
        mLogger.Debug() << "Got LMC: " << click << std::endl;
        if (Within(click))
        {
            RunDialog();
        }
    }

    void RightMousePress(glm::vec2 click) override
    {
        mLogger.Debug() << "Got RMC: " << click << std::endl;
        if (Within(click))
        {
        }
    }

private:

    BAK::DialogStore mDialogStore;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;
    const Font& mFont;
    Label mLabel;

    Widget mFullscreenFrame;
    Frame mActionAreaFrame;
    Widget mActionAreaBackground;
    Frame mLowerFrame;

    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;

    std::optional<BAK::Target> mCurrentTarget;
    std::optional<BAK::DialogSnippet> mCurrentDialog;

    const Logging::Logger& mLogger;
};

}
