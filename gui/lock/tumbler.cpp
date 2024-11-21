#include "gui/lock/tumbler.hpp"

#include "bak/lock.hpp"

#include "com/assert.hpp"

#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/fontManager.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

Tumbler::Tumbler(
    glm::vec2 pos,
    glm::vec2 dims,
    const Font& font)
:
    Widget{
        RectTag{},
        pos,
        dims,
        Color::tumblerBackground,
        true
    },
    mFont{font},
    mSelectedDigit{0},
    mDigits{},
    mCharacter{
        {0,0},
        dims
    }
{
    AddChildren();
}

void Tumbler::SetDigits(unsigned charIndex, const BAK::FairyChest& chest)
{
    for (unsigned i = 0; i < chest.mOptions.size(); i++)
    {
        ASSERT(charIndex < chest.mOptions[i].size());
        mDigits.emplace_back(chest.mOptions[i][charIndex]);
    }
    SetDigit(0);
}

void Tumbler::NextDigit()
{
    const auto nextDigit = (mSelectedDigit + 1) % mDigits.size();
    SetDigit(nextDigit);
}

char Tumbler::GetDigit() const
{
    return mDigits[mSelectedDigit];
}

void Tumbler::SetDigit(unsigned digitIndex)
{
    ASSERT(digitIndex < mDigits.size());
    mSelectedDigit = digitIndex;
    mCharacter.SetText(mFont, "\xf7" + std::string{mDigits[digitIndex]}, true, true);
}

void Tumbler::AddChildren()
{
    ClearChildren();
    AddChildBack(&mCharacter);
}


}
