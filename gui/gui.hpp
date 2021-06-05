#pragma once

#include <glm/glm.hpp>

#include <vector>

namespace Gui {

struct GuiElement
{
    bool mPressed;
    unsigned mImage;
    unsigned mPressedImage;
    glm::vec3 mPosition;
    glm::vec3 mDims;
};

class Gui
{
    void Render()
    {
        
    };

    std::vector<GuiElement> mElements;
};

}
