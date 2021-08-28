#pragma once

#include "bak/dialog.hpp"

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/frame.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

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
        const Font& fr)
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
            glm::vec2{15, 5},
            glm::vec2{220, 100}
        },
        mLowerFrame{
            glm::vec2{15, 120},
            glm::vec2{260, 76}
        },
        mFullscreenTextBox{
            glm::vec2{30, 30},
            glm::vec2{320 - 30*2, 135}},
        mActionAreaTextBox{
            glm::vec2{2, 2},
            glm::vec2{260, 100}},
        mLowerTextBox{
            glm::vec2{0, 0},
            glm::vec2{290, 70}},
        mLogger{Logging::LogState::GetLogger("Gui::DialogRunner")}
    {
        mFullscreenFrame.AddChildBack(&mFullscreenTextBox);
        mActionAreaFrame.AddChildBack(&mActionAreaTextBox);
        mLowerFrame.AddChildBack(&mLowerTextBox);
    }

    //const std::vector<Graphics::IGuiElement*>& GetChildren() const override
    //{
    //    return mActiveFrame;
    //}
    
    void AddText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredX,
        bool centeredY)
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
                centeredY);
            AddChildBack(&mFullscreenFrame);
        } break;
        case DialogFrame::ActionArea:
        {
            mActionAreaTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY);
            AddChildBack(&mActionAreaFrame);
        } break;
        case DialogFrame::LowerArea:
        {
            mLowerTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY);
            AddChildBack(&mLowerFrame);
        } break;
        }
    }

    void BeginDialog(BAK::Target target)
    {
        Logging::LogDebug("Gui::DialogRunner")
            << "BeginDialog " << target << "\n";

        bool progressing = true;
        auto current = target;
        mCurrentDialog = mDialogStore.GetSnippet(current);
        constexpr std::string_view empty = "";
        while (progressing)
        {
            const auto text = mCurrentDialog->GetText();
            if (text != empty) 
            {
                const auto ds1 = mCurrentDialog->mDisplayStyle;
                const auto act = mCurrentDialog->mActor;
                const auto dialogFrame = std::invoke([ds1, act]{
                    if (act != 0x0)
                        return DialogFrame::LowerArea;

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
                    = (ds2 & 0x10) == 0x10;
                const bool horizontallyCentered 
                    = (ds2 & 0x4) == 0x4;

                AddText(
                    text,
                    dialogFrame,
                    horizontallyCentered,
                    verticallyCentered);

                mLogger.Debug() << "Snippet: " << mCurrentDialog << "\n";

                return;
            }
            else
            {
                //if (mCurrentDialog->GetChoices().size() > 1)
                //    current = mCurrentDialog->GetChoices().back().mTarget;
                //else
                mLogger.Debug() << "Snippet: " << mCurrentDialog << "\n";
                if (mCurrentDialog && mCurrentDialog->GetChoices().size() >= 1)
                    current = mCurrentDialog->GetChoices().front().mTarget;
                else
                    current = BAK::KeyTarget{0x2dc6d4};
                mCurrentDialog = mDialogStore.GetSnippet(current);
            }
        }
    }

    void RightMousePress(glm::vec2 click) override
    {
        mLogger.Debug() << "Got LMC: " << click << std::endl;
        if (Within(click))
        {
            if (mCurrentDialog && mCurrentDialog->GetChoices().size() > 0)
            {
                BeginDialog(
                    mCurrentDialog->GetChoices().front().mTarget);
            }
            else
                BeginDialog(BAK::KeyTarget{0x2dc6d4});
        }
    }

private:

    BAK::DialogStore mDialogStore;
    glm::vec2 mCenter;
    const Font& mFont;
    Label mLabel;

    Widget mFullscreenFrame;
    Frame mActionAreaFrame;
    Frame mLowerFrame;

    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;

    std::optional<BAK::DialogSnippet> mCurrentDialog;

    const Logging::Logger& mLogger;
};

}
