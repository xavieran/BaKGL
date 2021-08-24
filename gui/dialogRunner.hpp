#pragma once

#include "bak/dialog.hpp"

#include "gui/widget.hpp"
#include "gui/colors.hpp"
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
        mLabel{
            mCenter,
            glm::vec2{320, 20},
            fr,
            "#LABEL#"},
        mTextBox{
            glm::vec2{2, 2},
            glm::vec2{260, 240}}
    {
        AddChildBack(&mLabel);
        AddChildBack(&mTextBox);
    }

    std::string_view GetText(BAK::Target tgt)
    {
        try
        {
            return mDialogStore.GetSnippet(tgt).GetText();
        }
        catch (const std::runtime_error& e)
        {
            return e.what();
        }
    }

    void BeginDialog(BAK::Target target)
    {
        Logging::LogDebug("Gui::DialogRunner") << "BeginDialog " << target << "\n";
        mTextBox.AddText(mLabel.mFont, GetText(target));
        mLabel.SetCenter(mCenter);
    }

    void LeftMousePress(glm::vec2 click) override
    {
        Logging::LogDebug("Gui::DialogRunner") << "Got LMC: " << click << std::endl;
        //if (Within(click))
        //{
        //    if (mLeftPressed)
        //        std::invoke(mLeftPressed);
        //}
    }


private:

    BAK::DialogStore mDialogStore;
    glm::vec2 mCenter;
    Label mLabel;
    TextBox mTextBox;
};

}
