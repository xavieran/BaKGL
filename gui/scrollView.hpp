#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
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
        Args&&... childArgs)
    :
        Widget{
            ClipRegionTag{},
            pos,
            dims,
            true 
        },
        mChild{std::forward<Args>(childArgs)...},
        mUp{
            glm::vec2{},
            glm::vec2{},
            mIcons.GetUpButton(),
            mIcons.GetUpButtonPressed(),
            [this]{ ScrollUp(); },
            []{}
        },
        mDown{
            glm::vec2{},
            glm::vec2{},
            mIcons.GetUpButton(),
            mIcons.GetUpButtonPressed(),
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

private:
    void ScrollUp()
    {
        mChild.AdjustPosition(glm::vec2{0, 10});
    }

    void ScrollDown()
    {
        mChild.AdjustPosition(glm::vec2{0, 10});
    }

    void AddChildren()
    {
        ClearChildren();
        AddChildBack(&mChild);
    }

    T mChild;
    ClickButtonImage mUp;
    ClickButtonImage mDown;
    const Logging::Logger& mLogger;
};

}
