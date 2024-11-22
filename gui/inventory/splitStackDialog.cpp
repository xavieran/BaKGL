#include "gui/inventory/splitStackDialog.hpp"

#include "gui/button.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <sstream>

namespace Gui {

SplitStackDialog::SplitStackDialog(
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
    mMaxAmount{0}
{
    ASSERT(!mTransfer);
    AddChildBack(&mFrame);
    AddChildBack(&mIncrease);
    AddChildBack(&mDecrease);
    AddChildBack(&mGive);
}

void SplitStackDialog::BeginSplitDialog(
    Callback&& callback,
    unsigned amount)
{
    mTransfer = std::move(callback);
    mAmount = amount;
    mMaxAmount = amount;
    UpdateAmountText();
}

void SplitStackDialog::Increase()
{
    if (++mAmount > mMaxAmount) mAmount = mMaxAmount;
    UpdateAmountText();
}

void SplitStackDialog::Decrease()
{
    if (mAmount > 1) mAmount--;
    UpdateAmountText();
}

void SplitStackDialog::UpdateAmountText()
{
    std::stringstream ss{};
    ss << "#Give: " << mAmount << "/" << mMaxAmount;
    mGive.SetText(ss.str());
}

void SplitStackDialog::TransferItem(bool share)
{
    ASSERT(mTransfer);
    mTransfer(share, mAmount);
}

}
