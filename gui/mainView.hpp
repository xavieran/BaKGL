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
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class MainView : public Widget
{
public:

    MainView(
        Graphics::SpriteManager& spriteManager)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            std::invoke([&spriteManager]{
                const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
                return sheetIndex;
            }),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mSpriteSheet{GetDrawInfo().mSpriteSheet},
        mCompass{},
        mButtons{},
        mSceneElements{},
        mLogger{Logging::LogState::GetLogger("Gui::MainView")}
    {
        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "FRAME.SCX", "Z01.PAL");

        const auto normalOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "BICONS1.BMX", "OPTIONS.PAL");

        const auto pressedOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "BICONS2.BMX", "OPTIONS.PAL");

        const auto headsOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "HEADS.BMX", "OPTIONS.PAL");

        const auto compassOffset = textures.size();
        BAK::TextureFactory::AddToTextureStore(
            textures, "COMPASS.BMX", "OPTIONS.PAL");

        auto [w, h] = textures.GetTexture(compassOffset).GetDims();
        mCompass.emplace(
            glm::vec2{144,121},
            glm::vec2{32,12},
            glm::vec2{w, h + 1},
            mSpriteSheet,
            static_cast<Graphics::TextureIndex>(compassOffset));

        RequestResource request{};
        {
            auto fb = FileBufferFactory::CreateFileBuffer("REQ_MAIN.DAT");
            request.Load(&fb);
        }

        mButtons.reserve(request.GetSize());
        mSceneElements.reserve(request.GetSize());

        for (unsigned i = 0; i < request.GetSize(); i++)
        {
            auto data = request.GetRequestData(i);
            mLogger.Debug() << data << "\n";
            switch (data.widget)
            {
            case REQ_USERDEFINED:
            {
                if (data.action == 192) break;
                int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
                int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();
                int w = request.GetRectangle().GetWidth();
                int h = request.GetRectangle().GetHeight();
                mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mSpriteSheet,
                    data.image + headsOffset,
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec2{x, y},
                    glm::vec2{data.width, data.height},
                    true);
            }
                break;
            case REQ_IMAGEBUTTON:
            {
                int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
                int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();

                mButtons.emplace_back(
                    glm::vec2{x, y},
                    glm::vec2{data.width, data.height},
                    mSpriteSheet,
                    Graphics::TextureIndex{static_cast<unsigned>(data.image + normalOffset)},
                    Graphics::TextureIndex{static_cast<unsigned>(data.image + pressedOffset)},
                    []{});
            }
                break;
            default:
                mLogger.Info() << "Unhandled: " << i << "\n";
                break;
            }
        }

        for (auto& action : mSceneElements)
            AddChildBack(&action);

        for (auto& button : mButtons)
            AddChildBack(&button);

        assert(mCompass);
        AddChildBack(&(*mCompass));

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

private:
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::optional<Compass> mCompass;
    std::vector<ClickButtonImage> mButtons;
    std::vector<Widget> mSceneElements;

    const Logging::Logger& mLogger;
};

}
