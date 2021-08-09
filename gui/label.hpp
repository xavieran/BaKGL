#pragma once

#include "graphics/IGuiElement.hpp"
#include "graphics/sprites.hpp"

#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class Label
{
public:

    Label(
        Graphics::SpriteManager& spriteManager)
    :
        mSpriteSheet{std::invoke([&spriteManager]{
            const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
            return sheetIndex;
        })},
        mGuiElement{
            Graphics::DrawMode::Sprite,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec3{0},
            glm::vec3{1},
            true
        },
        mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
    {
        //auto* action = new Graphics::IGuiElement{
        //    Graphics::DrawMode::Rect,
        //    mSpriteSheet,
        //    0, // no image index
        //    Graphics::ColorMode::SolidColor,
        //    sceneRect.mColor,
        //    sceneRect.mPosition,
        //    sceneRect.mDimensions,
        //    false};

        //mGuiElement.AddChild(elem);
    }

    void SetText(std::string_view text)
    {

    }

    Graphics::SpriteSheetIndex mSpriteSheet;
    SceneElement mGuiElement;

    const Logging::Logger& mLogger;
};

}
