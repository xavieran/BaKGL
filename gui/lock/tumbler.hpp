#pragma once

#include "bak/lock.hpp"
#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/inventory.hpp"
#include "bak/layout.hpp"
#include "bak/objectInfo.hpp"
#include "bak/textureFactory.hpp"

#include "gui/core/clickable.hpp"
#include "gui/core/dragEndpoint.hpp"
#include "gui/core/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class Tumbler :
    public Widget
{
public:
    Tumbler(
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
    
    void SetDigits(unsigned charIndex, const BAK::FairyChest& chest)
    {
        for (unsigned i = 0; i < chest.mOptions.size(); i++)
        {
            ASSERT(charIndex < chest.mOptions[i].size());
            mDigits.emplace_back(chest.mOptions[i][charIndex]);
        }
        SetDigit(0);
    }

    void NextDigit()
    {
        const auto nextDigit = (mSelectedDigit + 1) % mDigits.size();
        SetDigit(nextDigit);
    }

    char GetDigit() const
    {
        return mDigits[mSelectedDigit];
    }

private:
    void SetDigit(unsigned digitIndex)
    {
        ASSERT(digitIndex < mDigits.size());
        mSelectedDigit = digitIndex;
        mCharacter.AddText(mFont, "\xf7" + std::string{mDigits[digitIndex]}, true, true);
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mCharacter);
    }

    const Font& mFont;
    unsigned mSelectedDigit;
    std::vector<char> mDigits;

    TextBox mCharacter;
};

}
