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

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class MainView : public Widget
{
public:

    MainView(
        Graphics::SpriteManager& spriteManager,
        IGuiManager& guiManager)
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
        mHeadOffset{0},
        mCompass{},
        mButtons{},
        mCharacters{},
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

        mHeadOffset = textures.size();

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
        mCharacters.reserve(request.GetSize());

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
                mCharacterLocations.emplace_back(
                    glm::vec2{x, y},
                    glm::vec2{data.width, data.height});
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
                    []{},
                    []{});
            }
                break;
            default:
                mLogger.Info() << "Unhandled: " << i << "\n";
                break;
            }
        }

        AddChildren();
        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

    void SetHeading(BAK::GameHeading heading)
    {
        assert(mCompass);
        mCompass->SetHeading(heading);
    }

    void AddChildren()
    {
        for (auto& button : mButtons)
            AddChildBack(&button);

        assert(mCompass);
        AddChildBack(&(*mCompass));

        for (auto& action : mCharacters)
            AddChildBack(&action);
    }

    void UpdatePartyMembers(const BAK::GameState& gameState)
    {
        ClearChildren();

        mCharacters.clear();
        mCharacters.reserve(mCharacterLocations.size());

        const auto& party = gameState.GetParty();
        mLogger.Info() << "Updating Party: " << party<< "\n";
        for (unsigned person = 0; person < party.mActiveCharacters.size(); person++)
        {
            assert(person < mCharacterLocations.size());
            const auto& [pos, dims] = mCharacterLocations[person];
            const auto image = mHeadOffset + party.mActiveCharacters[person];
            mCharacters.emplace_back(
                pos,
                dims,
                mSpriteSheet,
                image,
                image,
                [this, character=party.mActiveCharacters[person]]{
                    ShowInventory(character);
                },
                [this, character=party.mActiveCharacters[person]]{
                    ShowPortrait(character);
                }
            );
        }

        AddChildren();
    }

    void ShowPortrait(unsigned character)
    {
        mGuiManager.ShowCharacterPortrait(character);
    }

    void ShowInventory(unsigned character)
    {
        mGuiManager.ShowInventory(character);
    }

private:
    IGuiManager& mGuiManager;
    Graphics::SpriteSheetIndex mSpriteSheet;
    unsigned mHeadOffset;
    std::optional<Compass> mCompass;
    std::vector<ClickButtonImage> mButtons;
    std::vector<std::pair<glm::vec2, glm::vec2>> mCharacterLocations;
    std::vector<ClickButtonImage> mCharacters;

    const Logging::Logger& mLogger;
};

}
