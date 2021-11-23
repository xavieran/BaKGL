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
    static constexpr auto sDims = glm::vec2{76, 38};
    using Callback = std::function<void(bool, unsigned)>;

    SplitStackDialog(
        glm::vec2 pos,
        const Font& font)
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
        mGive{
            {4, 4},
            {sDims.x - 9, 14},
            mFont,
            "Give",
            [this]{ TransferItem(false); }
        },
        mDecrease{
            {4, 20},
            {32, 14},
            mFont,
            "<",
            [this]{ Decrease(); }
        },
        mIncrease{
            {32 + 8, 20},
            {32, 14},
            mFont,
            ">",
            [this]{ Increase(); }
        },
        mTransfer{},
        mAmount{0},
        mMaxAmount{0},
        mLogger{Logging::LogState::GetLogger("Gui::SplitStackDialog")}
    {
        ASSERT(!mTransfer);
        AddChildBack(&mFrame);
        AddChildBack(&mIncrease);
        AddChildBack(&mDecrease);
        AddChildBack(&mGive);
    }

    void BeginSplitDialog(
        Callback&& callback,
        unsigned amount)
    {
        mTransfer = std::move(callback);
        mAmount = amount;
        mMaxAmount = amount;
        UpdateAmountText();
    }

private:
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

    void UpdateAmountText()
    {
        std::stringstream ss{};
        ss << "#Give: " << mAmount << "/" << mMaxAmount;
        mGive.SetText(ss.str());
    }

    void TransferItem(bool share)
    {
        ASSERT(mTransfer);
        mTransfer(share, mAmount);
    }

private:
    const Font& mFont;

    Button mFrame;

    ClickButton mGive;
    ClickButton mDecrease;
    ClickButton mIncrease;
    
    std::function<void(bool, unsigned)> mTransfer;

    unsigned mAmount;
    unsigned mMaxAmount;

    const Logging::Logger& mLogger;
};

}
