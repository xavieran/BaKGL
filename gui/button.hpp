#pragma once

#include "graphics/IGuiElement.hpp"

namespace Gui {

class Button: public Graphics::IGuiElement

{
public:
    Button(
        glm::vec3 pos,
        glm::vec3 dim)
    :
        // Bottom left edge
        Graphics::IGuiElement{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            glm::vec4{.333, .271, .173, 1},
            pos,
            dim,
            false 
        }
    {
        // Top Right edge
        this->AddChildBack(new
            Graphics::IGuiElement{
                Graphics::DrawMode::Rect,
                0,
                0,
                Graphics::ColorMode::SolidColor,
                glm::vec4{.651, .573, .255, 1},
                glm::vec3{1, 0, 0},
                dim - glm::vec3{1, 1, 0},
                false 
            });

        // Center
        this->AddChildBack(new
            Graphics::IGuiElement{
                Graphics::DrawMode::Rect,
                0,
                0,
                Graphics::ColorMode::SolidColor,
                glm::vec4{.604, .427, .220, 1},
                glm::vec3{1, 1, 0},
                dim - glm::vec3{2, 2, 0},
                false
            });
    }

};

}
