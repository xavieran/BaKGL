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
            glm::vec2{220, 110}
        },
        mLowerFrame{
            glm::vec2{15, 120},
            glm::vec2{260, 200}
        },
        mFullscreenTextBox{
            glm::vec2{28, 2},
            glm::vec2{320 - 26*2, 240}},
        mActionAreaTextBox{
            glm::vec2{2, 2},
            glm::vec2{260, 240}},
        mLowerTextBox{
            glm::vec2{2, 2},
            glm::vec2{260, 240}},
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
    //
    void AddText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredY)
    {
        switch (dialogFrame)
        {
        case DialogFrame::Fullscreen:
        {
            const auto dims = mFullscreenTextBox.AddText(
                mFont,
                text);
            const auto frameDims = mFullscreenFrame
                .GetPositionInfo()
                .mDimensions;
            const auto centerY = (frameDims.y / 2.0) - (dims.y / 2.0);
            const auto textPos = mFullscreenTextBox.GetPositionInfo().mPosition;
            mFullscreenTextBox.SetPosition(
                glm::vec2{
                    textPos.x,
                    centerY});
                    
            ClearChildren();
            AddChildBack(&mFullscreenFrame);
        } break;
        case DialogFrame::ActionArea:
        {
            mActionAreaTextBox.AddText(
                mFont,
                text);
            ClearChildren();
            AddChildBack(&mActionAreaFrame);
        } break;
        case DialogFrame::LowerArea:
        {
            mLowerTextBox.AddText(
                mFont,
                text);
            ClearChildren();
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
        auto dialogSnippet = mDialogStore.GetSnippet(current);
        constexpr std::string_view empty = "";
        while (progressing)
        {
            const auto text = dialogSnippet.GetText();
            if (text != empty) 
            {
                auto ds1 = dialogSnippet.mDisplayStyle;
                const auto dialogFrame = std::invoke([&ds1]{
                    if (ds1 == 0x02)
                        return DialogFrame::ActionArea;
                    else if (ds1 == 0x03
                        || ds1 == 0x04)
                        return DialogFrame::LowerArea;
                    else
                        return DialogFrame::Fullscreen;
                });

                AddText(
                    text,
                    dialogFrame,
                    false);

                mLogger.Debug() << "Snippet: " << dialogSnippet << "\n";

                return;
            }
            else
            {
                current = dialogSnippet.GetChoices()[0].mTarget;
                dialogSnippet = mDialogStore.GetSnippet(current);
            }
        }
    }

    void LeftMousePress(glm::vec2 click) override
    {
        mLogger.Debug() << "Got LMC: " << click << std::endl;
        if (Within(click))
        {
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

    const Logging::Logger& mLogger;
};

}
