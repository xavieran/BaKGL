#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/scene.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class MainView : public Graphics::IGuiElement
{
public:

    MainView(
        Graphics::SpriteManager& spriteManager)
    :
        Graphics::IGuiElement{
            Graphics::DrawMode::Sprite,
            std::invoke([&spriteManager]{
                const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
                return sheetIndex;
            }),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec3{0},
            glm::vec3{1}
        },
        mLogger{Logging::LogState::GetLogger("Gui::MainView")}
    {
        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "FRAME.SCX", "Z01.PAL");

        const auto iconsOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "BICONS1.BMX", "OPTIONS.PAL");

        const auto iconsOffset2 = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "BICONS2.BMX", "OPTIONS.PAL");

        const auto headsOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "HEADS.BMX", "OPTIONS.PAL");

        RequestResource request{};
        {
            auto fb = FileBufferFactory::CreateFileBuffer("REQ_MAIN.DAT");
            request.Load(&fb);
        }

        for (unsigned i = 0; i < request.GetSize(); i++)
        {
            auto data = request.GetRequestData(i);
            mLogger.Debug() << data << "\n";
            switch (data.widget)
            {
            case REQ_USERDEFINED:
            {
                if (data.action != 1) break;
                int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
                int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();
                mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mSpriteSheet,
                    data.image + headsOffset,
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec3{x, y, 0},
                    glm::vec3{1,});
            }
                break;
            case REQ_IMAGEBUTTON:
            {
                int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
                int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();
                mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mSpriteSheet,
                    data.image + iconsOffset,
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec3{x, y, 0},
                    glm::vec3{1,1,0});
            }
                break;
            default:
                mLogger.Info() << "Unhandled: " << i << "\n";
                break;
            }
        }

        int x = request.GetRectangle().GetXPos() + request.GetXOff();
        int y = request.GetRectangle().GetYPos() + request.GetYOff();
        mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mSpriteSheet,
                    headsOffset,
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec3{x + 14, y + 144, 0},
                    glm::vec3{1});
        mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mSpriteSheet,
                    headsOffset + 1,
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec3{x + 73, y + 144, 0},
                    glm::vec3{1});
        mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mSpriteSheet,
                    headsOffset + 4,
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec3{x + 132, y + 144, 0},
                    glm::vec3{1});

        for (auto& action : mSceneElements)
        {
            AddChildBack(&action);
        }

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

    std::vector<Graphics::IGuiElement> mSceneElements;

    const Logging::Logger& mLogger;
};

}
