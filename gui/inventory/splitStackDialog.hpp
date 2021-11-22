#pragma once

#include "gui/button.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>

namespace Gui {

class SplitStackDialog
    : public Widget
{
public:
    static constexpr auto sDims = glm::vec2{80, 50};

    SplitStackDialog(
        glm::vec2 pos,
        const Font& font,
        std::function<void(bool, unsigned)>&& transferFunction)
    :
        Widget{
            RectTag{},
            pos,
            sDims,
            glm::vec4{1.0, 0, 0, .3},
            true},
        mFont{font},
        mFrame{
            glm::vec2{0},
            sDims,
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow
        },
        mAmountText{
            glm::vec2{0},
            {sDims.x, 14}
        },
        mDecrease{
            {4, 15},
            {32, 14},
            mFont,
            "<",
            [this]{ Decrease(); }
        },
        mIncrease{
            {32 + 8, 15},
            {32, 14},
            mFont,
            ">",
            [this]{ Increase(); }
        },
        mGive{
            {4, 15 * 2},
            {32, 14},
            mFont,
            "Give",
            []{}
        },
        mShare{
            {32 + 8, 15 * 2},
            {32, 14},
            mFont,
            "Share",
            []{}
        },
        mTransfer{std::move(transferFunction)},
        mAmount{0},
        mMaxAmount{0},
        mLogger{Logging::LogState::GetLogger("Gui::SplitStackDialog")}
    {
        AddChildBack(&mFrame);
        AddChildBack(&mAmountText);
        AddChildBack(&mIncrease);
        AddChildBack(&mDecrease);
        AddChildBack(&mGive);
        AddChildBack(&mShare);
    }

    void Increase()
    {
        if (++mAmount > mMaxAmount) mAmount = mMaxAmount;
        UpdateAmountText();
    }

    void Decrease()
    {
        if (mAmount > 1) mAmount--;
        UpdateAmountText();
    }

    void SetMaxAmount(unsigned amount)
    {
        mAmount = amount;
        mMaxAmount = amount;
        UpdateAmountText();
    }

    void UpdateAmountText()
    {
        std::stringstream ss{};
        ss << mAmount << "/" << mMaxAmount;
        mAmountText.AddText(mFont, ss.str(), true, true, true);
    }

private:
    const Font& mFont;

    Button mFrame;

    TextBox mAmountText;
    ClickButton mDecrease;
    ClickButton mIncrease;
    ClickButton mGive;
    ClickButton mShare;
    
    std::function<void(bool, unsigned)> mTransfer;

    unsigned mAmount;
    unsigned mMaxAmount;

    const Logging::Logger& mLogger;
};

}
