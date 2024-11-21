#pragma once

#include "bak/types.hpp"

#include "gui/label.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Logging {
class Logger;
}

namespace Gui {

class Actors;
class Font;

class Portrait : public Widget
{
public:
    Portrait(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Font& font,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex verticalBorder,
        Graphics::TextureIndex horizontalBorder,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress);
    
    void SetCharacter(BAK::CharIndex character, std::string_view name);

private:
    const Actors& mActors;
    const Font& mFont;

    ClickButtonBase mClickButton;

    Widget mClipRegion;

    Widget mPortrait;

    Widget mLeftBorder;
    Widget mRightBorder;
    Widget mTopBorder;
    Widget mBottomBorder;

    Label mLabel;

    const Logging::Logger& mLogger;
};

}
