#pragma once

#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

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
        },
        mLogger{Logging::LogState::GetLogger("Gui::ScrollBar")}
    {
        AddChildren();
    }

    void SetDimensions(glm::vec2 dims) override
    {
        Widget::SetDimensions(dims);
        mBar.SetDimensions(glm::vec2{16, dims.y});
        mHandle.SetDimensions(mBar.GetDimensions() - glm::vec2{sMarginPixels * 2});
        SetScale(mScale);
    }

    void SetScale(float scale)
    {
        mScale = scale;
        mHandle.SetDimensions(
            glm::vec2{mHandle.GetDimensions().x,
                (mBar.GetDimensions().y - sMarginPixels * 2) * mScale});
    }

    bool OnMouseEvent(const MouseEvent& event) override
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

    void SetBarPosition(float position)
    {
        mHandle.SetPosition(
            glm::vec2{
            sMarginPixels,
            sMarginPixels + (mBar.GetDimensions().y - sMarginPixels * 2 - mHandle.GetDimensions().y) * position});
    }

private:

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mBar);
        AddChildBack(&mHandle);
        //AddChildBack(&mUp);
        //AddChildBack(&mDown);
    }

    const bool mScrollVertical;
    glm::vec2 mLastMousePos;
    bool mHandlePressed;
    float mScale;
    std::function<void(glm::vec2)> mAdjustScrollable;
    Button mBar;
    Button mHandle;
    ClickButtonImage mUp;
    ClickButtonImage mDown;
    const Logging::Logger& mLogger;
};

}
