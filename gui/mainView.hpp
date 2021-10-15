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
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/scene.hpp"
#include "gui/widget.hpp"

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
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("FRAME.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mGuiManager{guiManager},
        mIcons{icons},
        mCompass{},
        mButtons{},
        mCharacters{},
        mLogger{Logging::LogState::GetLogger("Gui::MainView")}
    {
        auto [ss, ti, dims] = icons.GetCompass();
        mCompass.emplace(
            glm::vec2{144,121},
            glm::vec2{32,12},
            glm::vec2{dims.x, dims.y + 1},
            ss,
            ti);
            
        RequestResource request{"REQ_MAIN.DAT"};

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
                mCharacterLocations.emplace_back(
                    glm::vec2{x, y},
                    glm::vec2{data.width, data.height});
            }
                break;
            case REQ_IMAGEBUTTON:
            {
                int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
                int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();

                assert(std::get<0>(icons.GetButton(data.image))
                    == std::get<0>(icons.GetPressedButton(data.image)));
                mButtons.emplace_back(
                    glm::vec2{x, y},
                    glm::vec2{data.width, data.height},
                    std::get<0>(icons.GetButton(data.image)),
                    std::get<1>(icons.GetButton(data.image)),
                    std::get<1>(icons.GetPressedButton(data.image)),
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

        for (auto& character : mCharacters)
            AddChildBack(&character);
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
            const auto [spriteSheet, image, dimss] = mIcons.GetCharacterHead(
                party.mActiveCharacters[person]);
            mCharacters.emplace_back(
                pos,
                dims,
                spriteSheet,
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
    const Icons& mIcons;
    std::optional<Compass> mCompass;
    std::vector<ClickButtonImage> mButtons;
    std::vector<std::pair<glm::vec2, glm::vec2>> mCharacterLocations;
    std::vector<ClickButtonImage> mCharacters;

    const Logging::Logger& mLogger;
};

}
