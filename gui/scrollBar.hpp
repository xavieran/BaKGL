#pragma once

#include "gui/button.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class Icons;

class ScrollBar : public Widget
{
    static constexpr auto sMarginPixels = 2;
public:
    ScrollBar(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        glm::vec2 barDim,
        bool scrollVertical,
        std::function<void(glm::vec2)>&& adjustScrollable);

    void SetDimensions(glm::vec2 dims) override;
    void SetScale(float scale);
    bool OnMouseEvent(const MouseEvent& event) override;
    void SetBarPosition(float position);
private:

    void AddChildren();

    const bool mScrollVertical;
    glm::vec2 mLastMousePos;
    bool mHandlePressed;
    float mScale;
    std::function<void(glm::vec2)> mAdjustScrollable;
    Button mBar;
    Button mHandle;
    ClickButtonImage mUp;
    ClickButtonImage mDown;
};

}
