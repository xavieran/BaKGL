#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <optional>

namespace Gui {

/*
 * I don't use this right now as I feel it introduces a bit
 * too much lag into the input.
 */

class DoubleClickStateMachine
{
    struct Event
    {
        float mClickTime;
        glm::vec2 mClickPos;
    };

public:
    DoubleClickStateMachine(
        std::function<void(glm::vec2)>&& singlePress,
        std::function<void(glm::vec2)>&& singleRelease,
        std::function<void(glm::vec2)>&& doublePress);

    void HandlePress(glm::vec2 click, float time);
    void HandleRelease(glm::vec2 click, float time);
    void UpdateTime(float time);

private:
    float mReleaseTimeout{.1};
    float mDoubleClickTimeout{.3};

    std::optional<Event> mPress;
    std::optional<Event> mRelease;

    std::function<void(glm::vec2)> mSinglePress;
    std::function<void(glm::vec2)> mSingleRelease;
    std::function<void(glm::vec2)> mDoublePress;
};

}
