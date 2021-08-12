#pragma once

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <unordered_map>

namespace Gui {

struct GuiElement
{
    GuiElement(
        bool pressed,
        bool highlighted,
        unsigned image,
        unsigned pressedImage,
        glm::vec3 position,
        glm::vec3 dims,
        glm::vec3 scale,
        std::function<void()>&& leftRelease,
        std::function<void()>&& rightRelease)
    :
        mPressed{pressed},
        mHighlighted{highlighted},
        mImage{image},
        mPressedImage{pressedImage},
        mPosition{position},
        mDims{dims},
        mScale{scale},
        mLeftReleased{std::move(leftRelease)},
        mRightReleased{std::move(rightRelease)}
    {
        assert(mLeftReleased);
        assert(mRightReleased);
    }

    bool Within(glm::vec3 click)
    {
        return Graphics::PointWithinRectangle(
            glm::vec2{click},
            glm::vec2{mPosition},
            glm::vec2{mDims});
    }

    void LeftMousePress(glm::vec3 click)
    {
        if (Within(click))
        {
            mPressed = true;
        }
    }

    void LeftMouseRelease(glm::vec3 click)
    {
        mPressed = false;
        if (Within(click))
            std::invoke(mLeftReleased);
    }

    void RightMousePress(glm::vec3 click)
    {
    }

    void RightMouseRelease(glm::vec3 click)
    {
        if (Within(click))
            std::invoke(mRightReleased);
    }

    void MouseMoved(glm::vec3 pos)
    {
        if (Within(pos))
            mHighlighted = true;
        else
            mHighlighted = false;
    }

    bool mPressed;
    bool mHighlighted;
    unsigned mImage;
    unsigned mPressedImage;
    glm::vec3 mPosition;
    glm::vec3 mDims;
    glm::vec3 mScale;
    const std::function<void()> mLeftReleased;
    const std::function<void()> mRightReleased;
};

class Frame
{
public:
    Frame(glm::vec3 pos, glm::vec3 dim)
    :
        mPosition{pos},
        mDims{dim},
        mChildren{}
    {
    }

    void LeftMousePress(glm::vec3 click)
    {
        for (auto& c : mChildren)
            c.LeftMousePress(click - mPosition);
    }

    void LeftMouseRelease(glm::vec3 click)
    {
        for (auto& c : mChildren)
            c.LeftMouseRelease(click - mPosition);
    }

    void RightMousePress(glm::vec3 click)
    {
        for (auto& c : mChildren)
            c.RightMousePress(click - mPosition);
    }

    void RightMouseRelease(glm::vec3 click)
    {
        for (auto& c : mChildren)
            c.RightMouseRelease(click - mPosition);
    }

    void MouseMoved(glm::vec3 pos)
    {
        for (auto& c : mChildren)
            c.MouseMoved(pos - mPosition);
    }

    void Render()
    {

    }

    glm::vec3 mPosition;
    glm::vec3 mDims;
    std::vector<GuiElement> mChildren;
};

}
