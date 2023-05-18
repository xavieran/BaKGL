#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/scrollBar.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

template <ImplementsWidget T>
class ScrollView : public Widget
{
    // FIXME: Implement horizontal scroll
    static constexpr auto sScrollSpeed = 10.0f;
public:
    template <typename ...Args>
    ScrollView(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        bool scrollHorizontal,
        bool scrollVertical,
        Args&&... childArgs)
    :
        Widget{
            ClipRegionTag{},
            pos,
            dims,
            true 
        },
        mScrollHorizontal{scrollHorizontal},
        mScrollVertical{scrollVertical},
        mLastMousePos{0},
        mChild{std::forward<Args>(childArgs)...},
        mScrollBar{
            glm::vec2{dims.x - 16, 0},
            glm::vec2{16, dims.y},
            icons,
            glm::vec2{16, dims.y},
            true,
            [this](const auto& adjustment){ Scroll(adjustment); }
        },
        mLogger{Logging::LogState::GetLogger("Gui::ScrollView")}
    {
        AddChildren();
    }

    T& GetChild()
    {
        return mChild;
    }

    bool OnMouseEvent(const MouseEvent& event)
    {
        // ... track mouse position
        if (std::holds_alternative<MouseMove>(event))
        {
            mLastMousePos = GetValue(event);
            return Widget::OnMouseEvent(event);
        }

        if (Within(mLastMousePos) &&
            std::holds_alternative<MouseScroll>(event))
        {
            Scroll(GetValue(event));
            return true;
        }
        else if (Within(GetValue(event)))
        {
            return Widget::OnMouseEvent(event);
        }

        return false;
    }
    
    void ResetScroll()
    {
        mScrollBar.SetScale(std::min(GetDimensions().y / mChild.GetDimensions().y, 1.0f));
        mChild.SetPosition(glm::vec2{});
        mScrollBar.SetBarPosition(0);
    }

private:
    void Scroll(glm::vec2 adjustment)
    {
        const auto scale = glm::vec2{mScrollHorizontal, mScrollVertical} * sScrollSpeed;
        auto newPosition = mChild.GetTopLeft() + adjustment * scale;
        const auto dimDiff = (GetDimensions().y - mChild.GetDimensions().y);

        if (newPosition.y > 0
            || newPosition.y < dimDiff)
        {
            if (adjustment.y > 0)
            {
                newPosition = glm::vec2{newPosition.x, 0};
            }
            else if (adjustment.y < 0)
            {
                newPosition = glm::vec2{newPosition.x, dimDiff};
            }
        }

        // No need for scrolling if the child fits in the scroll view
        if (mChild.GetDimensions().y < GetDimensions().y) return;

        mChild.SetPosition(newPosition);
        mScrollBar.SetBarPosition(mChild.GetTopLeft().y / dimDiff);
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mChild);
        AddChildBack(&mScrollBar);
    }

    // Ideally we'd just track the cursor..?
    const bool mScrollHorizontal;
    const bool mScrollVertical;
    glm::vec2 mLastMousePos;
    T mChild;
    ScrollBar mScrollBar;
    const Logging::Logger& mLogger;
};

}
