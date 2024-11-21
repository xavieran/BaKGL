#include "gui/scrollBar.hpp"

#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

ScrollBar::ScrollBar(
    glm::vec2 pos,
    glm::vec2 dims,
    const Icons& icons,
    glm::vec2 barDim,
    bool scrollVertical,
    std::function<void(glm::vec2)>&& adjustScrollable)
:
    Widget{
        ClipRegionTag{},
        pos,
        dims,
        true 
    },
    mScrollVertical{scrollVertical},
    mLastMousePos{},
    mHandlePressed{false},
    mScale{1.0},
    mAdjustScrollable{std::move(adjustScrollable)},
    mBar{
        glm::vec2{0},
        barDim,
        Color::buttonBackground,
        Color::buttonHighlight,
        Color::buttonShadow
    },
    mHandle{
        glm::vec2{sMarginPixels},
        barDim - glm::vec2{sMarginPixels * 2},
        Color::buttonBackground,
        Color::buttonHighlight,
        Color::buttonShadow
    },
    mUp{
        glm::vec2{0},
        std::get<glm::vec2>(icons.GetButton(1)),
        std::get<Graphics::SpriteSheetIndex>(icons.GetButton(1)),
        std::get<Graphics::TextureIndex>(icons.GetButton(1)),
        std::get<Graphics::TextureIndex>(icons.GetPressedButton(1)),
        []{ },//Scroll(glm::vec2{0, 1}); },
        []{}
    },
    mDown{
        glm::vec2{0, 16},
        std::get<glm::vec2>(icons.GetButton(1)),
        std::get<Graphics::SpriteSheetIndex>(icons.GetButton(1)),
        std::get<Graphics::TextureIndex>(icons.GetButton(1)),
        std::get<Graphics::TextureIndex>(icons.GetPressedButton(1)),
        []{}, //Scroll(glm::vec2{0, -1}); },
        []{}
    }
{
    AddChildren();
}

void ScrollBar::SetDimensions(glm::vec2 dims)
{
    Widget::SetDimensions(dims);
    mBar.SetDimensions(glm::vec2{16, dims.y});
    mHandle.SetDimensions(mBar.GetDimensions() - glm::vec2{sMarginPixels * 2});
    SetScale(mScale);
}

void ScrollBar::SetScale(float scale)
{
    mScale = scale;
    mHandle.SetDimensions(
        glm::vec2{mHandle.GetDimensions().x,
            (mBar.GetDimensions().y - sMarginPixels * 2) * mScale});
}

bool ScrollBar::OnMouseEvent(const MouseEvent& event)
{
    if (std::holds_alternative<LeftMousePress>(event)
        && Within(GetValue(event)))
    {
        mHandlePressed = true;
        return true;
    }
    else if (std::holds_alternative<LeftMouseRelease>(event))
    {
        mHandlePressed = false;
    }

    if (mHandlePressed && std::holds_alternative<MouseMove>(event))
    {
        mAdjustScrollable((mLastMousePos - GetValue(event)) / (mBar.GetDimensions().y * mScale));
        return true;
    }
    if (std::holds_alternative<MouseMove>(event))
    {
        mLastMousePos = GetValue(event);
    }

    return false;
}

void ScrollBar::SetBarPosition(float position)
{
    mHandle.SetPosition(
        glm::vec2{
        sMarginPixels,
        sMarginPixels + (mBar.GetDimensions().y - sMarginPixels * 2 - mHandle.GetDimensions().y) * position});
}

void ScrollBar::AddChildren()
{
    ClearChildren();
    AddChildBack(&mBar);
    AddChildBack(&mHandle);
    //AddChildBack(&mUp);
    //AddChildBack(&mDown);
}

}
