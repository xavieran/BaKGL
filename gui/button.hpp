#pragma once

#include "graphics/IGuiElement.hpp"
#include "gui/colors.hpp"

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
            Gui::Color::buttonShadow,
            pos,
            dim
        },
        mTopRightEdge{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Gui::Color::buttonHighlight,
            glm::vec3{1, 0, 0},
            dim - glm::vec3{1, 1, 0}
        },
        mCenter{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Gui::Color::buttonBackground,
            glm::vec3{1, 1, 0},
            dim - glm::vec3{2, 2, 0}
        }
    {
        // Top Right edge
        this->AddChildBack(&mTopRightEdge);
        this->AddChildBack(&mCenter);
    }

    Graphics::IGuiElement mTopRightEdge;
    Graphics::IGuiElement mCenter;
};

}
