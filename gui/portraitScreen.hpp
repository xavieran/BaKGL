#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/colors.hpp"
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"
#include "gui/skills.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class PortraitScreen : public Widget
{
public:

    PortraitScreen(
        Graphics::SpriteManager& spriteManager,
        IGuiManager& guiManager,
        const Font& mFont)
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
        mGuiManager{guiManager},
        mSpriteSheet{GetDrawInfo().mSpriteSheet},
        mElements{},
        mSkills{},
        mLogger{Logging::LogState::GetLogger("Gui::PortraitScreen")}
    {
        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "DIALOG.SCX", "INVENTOR.PAL");

        const auto normalOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "INVSHP1.BMX", "INVENTOR.PAL");

        const auto pressedOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "INVSHP2.BMX", "INVENTOR.PAL");

        RequestResource request{};
        {
            auto fb = FileBufferFactory::CreateFileBuffer("REQ_INFO.DAT");
            request.Load(&fb);
        }

        mElements.reserve(request.GetSize());

        {
            unsigned i = 1; // exit
            const auto& data = request.GetRequestData(i);
            int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
            int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();
            mButtons.emplace_back(
                glm::vec2{x, y},
                glm::vec2{data.width, data.height},
                mFont,
                "Exit",
                [this]{ mGuiManager.ExitCharacterPortrait(); });
        }

        {
            unsigned i = 0; // portrait
            const auto& data = request.GetRequestData(i);
            int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
            int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();
            mElements.emplace_back(
                Graphics::DrawMode::Sprite,
                mSpriteSheet,
                pressedOffset + 25,
                Graphics::ColorMode::Texture,
                Color::black,
                glm::vec2{x, y},
                glm::vec2{4, data.height},
                true);
        mElements.emplace_back(
                Graphics::DrawMode::Sprite,
                mSpriteSheet,
                pressedOffset + 26,
                Graphics::ColorMode::Texture,
                Color::black,
                glm::vec2{x, y},
                glm::vec2{222, 4},
                true);

        }

        mSkills.emplace(
            mSpriteSheet,
            pressedOffset,
            mFont,
            request);

        AddChildren();
        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

    void AddChildren()
    {
        for (auto& elem : mElements)
            AddChildBack(&elem);

        for (auto& but : mButtons)
            AddChildBack(&but);

        assert(mSkills);
        AddChildBack(&(*mSkills));
    }

private:
    IGuiManager& mGuiManager;
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::vector<Widget> mElements;
    std::vector<ClickButton> mButtons;
    std::optional<Skills> mSkills;

    const Logging::Logger& mLogger;
};

}
