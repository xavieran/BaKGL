#pragma once

#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Logging {
class Logger;
}

namespace BAK {
class Character;
}

namespace Gui {

class Font;

class Ratings : public Widget
{
public:
    Ratings(
        glm::vec2 pos,
        glm::vec2 dims,
        const Font& font,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex verticalBorder,
        Graphics::TextureIndex horizontalBorder);

    void SetCharacter(const BAK::Character& character);

private:
    const Font& mFont;

    Widget mClipRegion;

    Widget mRatings;
    TextBox mRatingsText;
    TextBox mConditionsText;

    Widget mLeftBorder;
    Widget mRightBorder;
    Widget mTopBorder;
    Widget mBottomBorder;

    const Logging::Logger& mLogger;
};

}
