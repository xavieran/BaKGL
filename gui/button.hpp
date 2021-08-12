#pragma once

#include "gui/fixedGuiElement.hpp"
#include "gui/colors.hpp"

namespace Gui {

class Button: public FixedGuiElement

{
public:
    Button(
        glm::vec3 pos,
        glm::vec3 dim)
    :
        // Bottom left edge
        FixedGuiElement{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Gui::Color::buttonShadow,
            pos,
            dim,
            true
        },
        mTopRightEdge{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Gui::Color::buttonHighlight,
            glm::vec3{1, 0, 0},
            dim - glm::vec3{1, 1, 0},
            true
        },
        mCenter{
            Graphics::DrawMode::Rect,
            0,
            0,
            Graphics::ColorMode::SolidColor,
            Gui::Color::buttonBackground,
            glm::vec3{1, 1, 0},
            dim - glm::vec3{2, 2, 0},
            true
        }
    {
        // Top Right edge
        this->AddChildBack(&mTopRightEdge);
        this->AddChildBack(&mCenter);
    }

    FixedGuiElement mTopRightEdge;
    FixedGuiElement mCenter;
};

}
