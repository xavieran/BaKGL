#pragma once

#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/centeredImage.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace BAK {
class GameState;
class InventoryItem;
}

namespace Gui {

class Icons;
class Font;

class Details : public Widget
{
public:
    Details(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        const Font& font,
        std::function<void()>&& finished);
    
    bool OnMouseEvent(const MouseEvent& event);
    void AddItem(const BAK::InventoryItem& item, BAK::GameState& gameState);
private:
    bool HasMoreInfo(const BAK::InventoryItem& item);
    void ShowMoreInfo();
    std::string MakeMoreInfo(const BAK::InventoryItem& item);
    bool MousePressed();
    void AddChildren();

    const Icons& mIcons;
    const Font& mFont;
    TextBox mName;
    CenteredImage mItem;
    TextBox mStatusText;
    Button mDescriptionBackground;
    TextBox mDescriptionText;
    ClickButton mMoreInfo;
    bool mHasMoreInfo;
    bool mShowingMoreInfo;
    std::string mMoreInfoDescription;
    std::string mDescription;
    std::function<void()> mFinished;
};

}
