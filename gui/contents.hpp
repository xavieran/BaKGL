#pragma once

#include "bak/textureFactory.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GenericRequestScreen: public Widget
{
public:

    GenericRequestScreen(
        Graphics::SpriteManager& spriteManager,
        std::string_view palette,
        std::string_view request,
        std::string_view screen)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            spriteManager.AddSpriteSheet(),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mSpriteSheet{GetDrawInfo().mSpriteSheet},
        mButtons{},
        mLogger{Logging::LogState::GetLogger("Gui::GenericRequestScreen")}
    {
        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, screen, palette);

        const auto normalOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "BICONS1.BMX", palette);

        const auto pressedOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "BICONS2.BMX", palette);

        RequestResource requestRes{};
        {
            auto fb = FileBufferFactory::CreateDataBuffer(
                std::string{request});
            requestRes.Load(&fb);
        }

        mButtons.reserve(requestRes.GetSize());

        for (unsigned i = 0; i < requestRes.GetSize(); i++)
        {
            auto data = requestRes.GetRequestData(i);
            mLogger.Debug() << data << "\n";
            switch (data.widget)
            {
            case REQ_IMAGEBUTTON:
            {
                int x = data.xpos + requestRes.GetRectangle().GetXPos() + requestRes.GetXOff();
                int y = data.ypos + requestRes.GetRectangle().GetYPos() + requestRes.GetYOff();

                mButtons.emplace_back(
                    glm::vec2{x, y},
                    glm::vec2{data.width, data.height},
                    mSpriteSheet,
                    Graphics::TextureIndex{static_cast<unsigned>(data.image + normalOffset)},
                    Graphics::TextureIndex{static_cast<unsigned>(data.image + pressedOffset)},
                    []{},
                    []{});
            }
                break;
            default:
                mLogger.Info() << "Unhandled: " << i << "\n";
                break;
            }
        }

        for (auto& button : mButtons)
            AddChildBack(&button);

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::vector<ClickButtonImage> mButtons;

    const Logging::Logger& mLogger;
};

}
