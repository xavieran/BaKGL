#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/algorithm.hpp"

#include "gui/actors.hpp"
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
        const Actors& actors,
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
        mCenter{148, 113},
        mFont{fr},
        mActors{actors},
        mLabel{
            mCenter,
            glm::vec2{320, 20},
            mFont,
            "#LABEL#"},
        mActor{
            Graphics::DrawMode::Sprite,
            mActors.GetSpriteSheet(),
            0,
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{100, 19},
            glm::vec2{0},
            true},
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
            glm::vec2{15, 125},
            glm::vec2{285, 66}
        },
        mFullscreenTextBox{
            glm::vec2{30, 30},
            glm::vec2{320 - 30*2, 135}
        },
        mActionAreaTextBox{
            glm::vec2{4, 4},
            glm::vec2{290, 103}
        },
        mLowerTextBox{
            glm::vec2{0, 0},
            glm::vec2{285, 66}
        },
        mCurrentTarget{},
        mCurrentDialog{},
        mRemainingText{""},
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
    
    auto AddText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredX,
        bool centeredY,
        bool isBold,
        std::optional<unsigned> actor)
    {
        ClearChildren();
        auto addTextResult = std::invoke([&]
        {
            switch (dialogFrame)
            {
            case DialogFrame::Fullscreen:
            {
                AddChildBack(&mFullscreenFrame);
                return mFullscreenTextBox.AddText(
                    mFont,
                    text,
                    centeredX,
                    centeredY,
                    isBold);
            } break;
            case DialogFrame::ActionArea:
            {
                AddChildBack(&mActionAreaFrame);
                return mActionAreaTextBox.AddText(
                    mFont,
                    text,
                    centeredX,
                    centeredY,
                    isBold);
            } break;
            case DialogFrame::LowerArea:
            {
                AddChildBack(&mLowerFrame);
                return mLowerTextBox.AddText(
                    mFont,
                    text,
                    centeredX,
                    centeredY,
                    isBold);
            } break;
            default:
                throw std::runtime_error("Invalid DialogArea");
            }
        });

        if (actor)
        {
            const auto& [
                index,
                dims] = mActors.GetActor(*actor);
            mActor.SetTexture(index);
            // we want the bottom of this picture to end up here
            mActor.SetPosition(glm::vec2{100, 112 - dims.y});
            mActor.SetDimensions(dims);
            AddChildBack(&mActor);
        }

        return addTextResult;
    }

    void UpdateSnippet()
    {
        std::string text;
        if (!mRemainingText.empty())
            text = std::string{mRemainingText};
        else
            text = std::string{mCurrentDialog->GetText()};

        text = std::regex_replace(
            text,
            std::regex{"@4"},
            std::string{mGameState.GetPartyLeader().mName});

        text = std::regex_replace(
            text,
            std::regex{"@0"},
            std::string{mGameState.GetPartyFollower().mName});

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

        std::optional<unsigned> actor{};

        if (act != 0)
        {
            actor = act;
            if (act == 0xff)
                actor = mGameState.GetPartyLeader().mIndex;
        }

        const auto [charPos, remainingText] = AddText(
            text,
            dialogFrame,
            horizontallyCentered,
            verticallyCentered,
            isBold,
            actor);

        mRemainingText = std::string{remainingText};
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
                else if (choiceState == BAK::ChoiceState::ShopType
                    && mGameState.GetShopType() == c.mChoice1)
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
        mRemainingText = "";
        Logging::LogDebug("Gui::DialogRunner")
            << "BeginDialog" << target << " snip: " << mCurrentDialog << "\n";
        RunDialog(true);
    }

    void AddLabel(std::string_view text)
    {
        mLabel.SetText(text);
        mLabel.SetCenter(mCenter);
        AddChildBack(&mLabel);
    }

    void ShowFlavourText(BAK::Target target)
    {

        mCurrentTarget = target;
        mCurrentDialog = mDialogStore.GetSnippet(target);
        const auto text = mCurrentDialog->GetText();
        const auto flavourText = find_nth(text.begin(), text.end(), '#', 2);
        assert(flavourText != text.end());
        mRemainingText = std::string{flavourText, text.end()};
        UpdateSnippet();

        auto label = std::string{text.begin(), flavourText};
        AddLabel(label);

        Logging::LogDebug("Gui::DialogRunner")
            << "ShowFlavourText" << target << " snip: " << mCurrentDialog << "\n";
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
            auto current       = ProgressDialog();
            auto currentDialog = mDialogStore.GetSnippet(current);
            mLogger.Debug() << "Progressed through: " << current 
                << "(" << mCurrentTarget << ") " << currentDialog << std::endl;
            if (current == *mCurrentTarget && !first)
            {
                progressing = false;
                return false;
            }

            mCurrentTarget = current;
            mCurrentDialog = currentDialog;

            const auto text = currentDialog.GetText();
            if (text != empty)
            {
                UpdateSnippet();
                progressing = false;
                return true;
            }

        } while (progressing && (iters++ < 20));

        return true;
    }

private:

    BAK::DialogStore mDialogStore;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;
    const Font& mFont;
    const Actors& mActors;
    Label mLabel;
    Widget mActor;

    Widget mFullscreenFrame;
    Frame mActionAreaFrame;
    Widget mActionAreaBackground;
    Frame mLowerFrame;

    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;

    std::optional<BAK::Target> mCurrentTarget;
    std::optional<BAK::DialogSnippet> mCurrentDialog;
    std::string mRemainingText;

    const Logging::Logger& mLogger;
};

}
