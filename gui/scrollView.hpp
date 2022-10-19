#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

template <ImplementsWidget T>
class ScrollView : public Widget
{
public:
    template <typename ...Args>
    ScrollView(
        glm::vec2 pos,
        glm::vec2 dims,
        const Icons& icons,
        Args&&... childArgs)
    :
        Widget{
            ClipRegionTag{},
            pos,
            dims,
            true 
        },
        mLastMousePos{0},
        mChild{std::forward<Args>(childArgs)...},
        mUp{
            glm::vec2{},
            std::get<glm::vec2>(icons.GetButton(1)),
            std::get<Graphics::SpriteSheetIndex>(icons.GetButton(1)),
            std::get<Graphics::TextureIndex>(icons.GetButton(1)),
            std::get<Graphics::TextureIndex>(icons.GetPressedButton(1)),
            [this]{ ScrollUp(); },
            []{}
        },
        mDown{
            glm::vec2{0, 10},
            std::get<glm::vec2>(icons.GetButton(1)),
            std::get<Graphics::SpriteSheetIndex>(icons.GetButton(1)),
            std::get<Graphics::TextureIndex>(icons.GetButton(1)),
            std::get<Graphics::TextureIndex>(icons.GetPressedButton(1)),
            [this]{ ScrollDown(); },
            []{}
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
        }

        if (Within(mLastMousePos) &&
            std::holds_alternative<MouseScroll>(event))
        {
            mChild.AdjustPosition(GetValue(event) * 16.f);
            return true;
        }
        else if (Within(GetValue(event)))
        {
            return Widget::OnMouseEvent(event);
        }

        return false;
    }

private:
    void ScrollUp()
    {
        mChild.AdjustPosition(glm::vec2{0, 16});
    }

    void ScrollDown()
    {
        mChild.AdjustPosition(glm::vec2{0, -16});
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mChild);
        AddChildBack(&mUp);
        AddChildBack(&mDown);
    }

    // Ideally we'd just track the cursor..?
    glm::vec2 mLastMousePos;
    T mChild;
    ClickButtonImage mUp;
    ClickButtonImage mDown;
    const Logging::Logger& mLogger;
};

}
