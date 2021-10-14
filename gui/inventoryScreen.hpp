#pragma once

#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/actors.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/portrait.hpp"
#include "gui/ratings.hpp"
#include "gui/widget.hpp"
#include "gui/skills.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class InventoryScreen : public Widget
{
public:

    InventoryScreen(
        Graphics::SpriteManager& spriteManager,
        IGuiManager& guiManager,
        const Font& font,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            spriteManager.AddSpriteSheet(),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200}, true
        },
        mGuiManager{guiManager},
        mFont{font},
        mGameState{gameState},
        mDialogScene{},
        mSelectedCharacter{0},
        mSpriteSheet{GetDrawInfo().mSpriteSheet},
        mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
    {
        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "INVENTOR.SCX", "INVENTOR.PAL");

        const auto normalOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "INVSHP1.BMX", "INVENTOR.PAL");

        const auto pressedOffset = textures.size();

        BAK::TextureFactory::AddToTextureStore(
            textures, "INVSHP2.BMX", "INVENTOR.PAL");

        RequestResource request{};
        {
            auto fb = FileBufferFactory::CreateFileBuffer("REQ_INV.DAT");
            request.Load(&fb);
        }

        AddChildren();
        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

    void SetSelectedCharacter(unsigned character)
    {
        mSelectedCharacter = character;
    }

    void AddChildren()
    {
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;
    unsigned mSelectedCharacter;
    Graphics::SpriteSheetIndex mSpriteSheet;
    const Logging::Logger& mLogger;
};

}
