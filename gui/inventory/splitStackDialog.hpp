#pragma once

#include "gui/button.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {
class Font;

class SplitStackDialog
    : public Widget
{
public:
    static constexpr auto sDims = glm::vec2{76, 38};
    using Callback = std::function<void(bool, unsigned)>;

    SplitStackDialog(
        glm::vec2 pos,
        const Font& font);
    
    void BeginSplitDialog(
        Callback&& callback,
        unsigned amount);

private:
    void Increase();
    void Decrease();
    void UpdateAmountText();
    void TransferItem(bool share);

private:
    const Font& mFont;

    Button mFrame;

    ClickButton mGive;
    ClickButton mDecrease;
    ClickButton mIncrease;
    
    std::function<void(bool, unsigned)> mTransfer;

    unsigned mAmount;
    unsigned mMaxAmount;
};

}
