#pragma once

#include "bak/lock.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
struct FairyChest;
}

namespace Gui {

class Font;

class Tumbler :
    public Widget
{
public:
    Tumbler(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font);
    
    void SetDigits(unsigned charIndex, const BAK::FairyChest& chest);
    void NextDigit();
    char GetDigit() const;
private:
    void SetDigit(unsigned digitIndex);
    void AddChildren();

    const Font& mFont;
    unsigned mSelectedDigit;
    std::vector<char> mDigits;

    TextBox mCharacter;
};

}
