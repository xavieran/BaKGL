#pragma once

#include "bak/dialog.hpp"

#include "gui/colors.hpp"
#include "gui/frame.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

namespace Gui {

class DialogRunner : public Widget
{
public:
    DialogRunner(
        glm::vec2 pos,
        glm::vec2 dims,
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
            glm::vec2{0, 0},
            glm::vec2{320, 240}
        },
        mActionAreaFrame{
            glm::vec2{15, 5},
            glm::vec2{220, 110}
        },
        mLowerFrame{
            glm::vec2{15, 120},
            glm::vec2{320, 240}
        },
        mActiveFrame{},
        mFullscreenTextBox{
            glm::vec2{2, 2},
            glm::vec2{260, 240}},
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
        mActiveFrame.reserve(1);
    }

    //const std::vector<Graphics::IGuiElement*>& GetChildren() const override
    //{
    //    return mActiveFrame;
    //}

    void BeginDialog(BAK::Target target)
    {
        Logging::LogDebug("Gui::DialogRunner") << "BeginDialog " << target << "\n";
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
                if (ds1 == 0x02)
                {
                    mActionAreaTextBox.AddText(
                        mFont,
                        text);
                    ClearChildren();
                    AddChildBack(&mActionAreaFrame);
                }
                else if (ds1 == 0x03
                      || ds1 == 0x04)
                {
                    mLowerTextBox.AddText(
                        mFont,
                        text);
                    ClearChildren();
                    AddChildBack(&mLowerFrame);
                }
                else
                {
                    mFullscreenTextBox.AddText(
                        mFont,
                        text);
                    ClearChildren();
                    AddChildBack(&mFullscreenFrame);
                }

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

    Frame mFullscreenFrame;
    Frame mActionAreaFrame;
    Frame mLowerFrame;

    std::vector<Graphics::IGuiElement*> mActiveFrame;
         
    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;

    const Logging::Logger& mLogger;
};

}
