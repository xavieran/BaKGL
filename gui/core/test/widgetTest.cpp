#include "gtest/gtest.h"

#include "com/logger.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui::Test {

struct TestWidget : public Widget
{

    template <typename ...Args>
    TestWidget(Args&&... args)
    :
        Widget{std::forward<Args>(args)...}
    {}

    bool OnMouseEvent(
        const MouseEvent& event) override
    {
        mMouseEvents.emplace_back(event);
        return Widget::OnMouseEvent(event);
    }

    void PropagateUp(
        const DragEvent& event) override
    {
        mDragEventsUp.emplace_back(event);
        return Widget::PropagateUp(event);
    }

    bool OnDragEvent(
        const DragEvent& event) override
    {
        mDragEventsDown.emplace_back(event);
        return Widget::OnDragEvent(event);
    }

    std::vector<DragEvent> mDragEventsUp;
    std::vector<DragEvent> mDragEventsDown;
    std::vector<MouseEvent> mMouseEvents;
};

struct WidgetTestFixture : public ::testing::Test
{
    WidgetTestFixture()
    :
        mRoot{
            RectTag{},
            glm::vec2{10, 10},
            glm::vec2{50, 50},
            glm::vec4{},
            true
        },
        mChild1{
            RectTag{},
            glm::vec2{2, 2},
            glm::vec2{10, 10},
            glm::vec4{},
            true
        },
        mChild2{
            RectTag{},
            glm::vec2{5, 5},
            glm::vec2{10, 10},
            glm::vec4{},
            true
        }
    {}

protected:
    void SetUp() override
    {
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);
        mRoot.AddChildBack(&mChild1);
        mChild1.AddChildBack(&mChild2);
    }

    TestWidget mRoot;
    TestWidget mChild1;
    TestWidget mChild2;
};

TEST_F(WidgetTestFixture, RelativeMouseEventPropagation)
{
    const auto event = MouseEvent{LeftMousePress{glm::vec2{20, 20}}};
    bool handled = mRoot.OnMouseEvent(event);

    EXPECT_EQ(handled, false);

    ASSERT_EQ(mRoot.mMouseEvents.size(), 1);
    EXPECT_EQ(mRoot.mMouseEvents.back(), event);

    ASSERT_EQ(mChild1.mMouseEvents.size(), 1);
    EXPECT_EQ(
        GetValue(mChild1.mMouseEvents.back()),
        (GetValue(event) - mRoot.GetPositionInfo().mPosition));

    ASSERT_EQ(mChild2.mMouseEvents.size(), 1);
    EXPECT_EQ(
        GetValue(mChild2.mMouseEvents.back()),
        (GetValue(event) 
             - mRoot.GetPositionInfo().mPosition
             - mChild1.GetPositionInfo().mPosition));
}

TEST_F(WidgetTestFixture, DragEventPropagationUp)
{
    const auto event    = DragEvent{DragStarted{&mChild2, glm::vec2{8, 8}}};
    const auto expected = DragEvent{DragStarted{&mChild2, glm::vec2{20, 20}}};
    mChild2.PropagateUp(event);

    ASSERT_EQ(mRoot.mDragEventsUp.size(), 1);
    EXPECT_EQ(mRoot.mDragEventsUp.back(), expected);
}

TEST_F(WidgetTestFixture, RelativeDragEventPropagation)
{
    const auto event = DragEvent{DragStarted{&mChild2, glm::vec2{20, 20}}};
    bool handled = mRoot.OnDragEvent(event);

    EXPECT_EQ(handled, false);

    ASSERT_EQ(mRoot.mDragEventsDown.size(), 1);
    EXPECT_EQ(mRoot.mDragEventsDown.back(), event);

    ASSERT_EQ(mChild1.mDragEventsDown.size(), 1);
    EXPECT_EQ(
        GetValue(mChild1.mDragEventsDown.back()),
        (GetValue(event) - glm::vec2{10, 10}));

    ASSERT_EQ(mChild2.mDragEventsDown.size(), 1);
    EXPECT_EQ(
        GetValue(mChild2.mDragEventsDown.back()),
        (GetValue(event) - glm::vec2{10, 10} - glm::vec2{2, 2}));
}

}  // namespace
