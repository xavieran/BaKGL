#pragma once

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

template <typename T>
concept ImplementsWidget = std::derived_from<T, Widget>;

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
        mElements.reserve(expectedSize);
    }

    template <typename ...Args>
    void AddWidget(Args&&... args)
    {
        auto& widget = mElements.emplace_back(std::forward<Args>(args)...);
        widget.SetPosition(mEnd);
        mEnd += widget.GetPositionInfo().mDimensions * mMultiplier;
        mEnd += mSpacing * mMultiplier;

        AddChildren();
    }

    void ClearWidgets()
    {
        ClearChildren();
        mElements.clear();
        mEnd = glm::vec2{0, 0};
    }

private:
    void AddChildren()
    {
        ClearChildren();
        for (auto& widget : mElements)
        {
            AddChildBack(&widget);
        }
    }

    glm::vec2 mMultiplier;
    float mSpacing;
    glm::vec2 mEnd;
    std::vector<T> mElements;
    const Logging::Logger& mLogger;
};

}
