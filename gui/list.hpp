#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

template <ImplementsWidget T>
class List : public Widget
{
public:
    List(
        std::size_t expectedSize,
        float spacing,
        bool isVerticallyAligned)
    :
        Widget{
            RectTag{},
            glm::vec2{0, 0},
            glm::vec2{0, 0},
            Color::black,
            true 
        },
        mMultiplier{
            isVerticallyAligned ? 0 : 1,
            isVerticallyAligned ? 1 : 0},
        mSpacing{spacing},
        mEnd{0, 0},
        mElements{},
        mLogger{Logging::LogState::GetLogger("Gui::List")}
    {
        //mElements.reserve(expectedSize);
    }

    template <typename ...Args>
    void AddWidget(Args&&... args)
    {
        ClearChildren();
        auto& widget = mElements.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        widget->SetPosition(widget->GetPositionInfo().mPosition + mEnd);
        mEnd += widget->GetPositionInfo().mDimensions * mMultiplier;
        mEnd += mSpacing * mMultiplier;
        AddChildren();
        SetDimensions(mEnd + glm::vec2{widget->GetPositionInfo().mDimensions.x, 0});
    }

    void ClearWidgets()
    {
        ClearChildren();
        mElements.clear();
        mEnd = glm::vec2{0, 0};
        SetDimensions(mEnd);
    }

private:
    void AddChildren()
    {
        ClearChildren();
        for (auto& widget : mElements)
        {
            AddChildBack(widget.get());
        }
    }

    glm::vec2 mMultiplier;
    float mSpacing;
    glm::vec2 mEnd;
    // Because mElements can grow, make these unique_ptrs so that 
    // they remain stable when mElements grows and they are moved.
    std::vector<std::unique_ptr<T>> mElements;
    const Logging::Logger& mLogger;
};

}
