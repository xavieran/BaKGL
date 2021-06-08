#pragma once

#include "graphics/glm.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace Gui {

struct GuiElement
{
    bool IsClicked(glm::vec3 click)
    {
        return Graphics::PointWithinRectangle(
            glm::vec2{click},
            glm::vec2{mPosition},
            glm::vec2{mDims});
    }

    void MousePress(glm::vec3 click)
    {
        if (IsClicked(click))
        {
            mPressed = true;
        }
    }

    void MouseRelease(glm::vec3 click)
    {
        mPressed = false;
    }

    void MouseMoved(glm::vec3 pos)
    {
        if (IsClicked(pos))
            mPressed = true;
        else
            mPressed = false;
    }

    unsigned mAction;
    bool mPressed;
    unsigned mImage;
    unsigned mPressedImage;
    glm::vec3 mPosition;
    glm::vec3 mDims;
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

    void MousePress(glm::vec3 click)
    {
        for (auto& c : mChildren)
            c.MousePress(click - mPosition);
    }

    void MouseRelease(glm::vec3 click)
    {
        for (auto& c : mChildren)
            c.MouseRelease(click - mPosition);
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

class ScreenFactory
{
    
};

class GDSScreen
{
    std::string_view mScreen  = "DIALOG.SCX";
    std::string_view mBackground = "G_NORTHW.BMX";
};

}


