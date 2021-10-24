#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/layout.hpp"
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
    static constexpr auto sLayoutFile = "REQ_MAIN.DAT";

    static constexpr auto sCharacterWidgetBegin = 10;

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
        mLayout{sLayoutFile},
        mCompass{
            glm::vec2{144,121},
            glm::vec2{32,12},
            std::get<glm::vec2>(icons.GetCompass())
                + glm::vec2{0, 1},
            std::get<Graphics::SpriteSheetIndex>(icons.GetCompass()),
            std::get<Graphics::TextureIndex>(icons.GetCompass())
        },
        mButtons{},
        mCharacters{},
        mLogger{Logging::LogState::GetLogger("Gui::MainView")}
    {
        mButtons.reserve(mLayout.GetSize());

        for (unsigned i = 0; i < mLayout.GetSize(); i++)
        {
            const auto& widget = mLayout.GetWidget(i);
            switch (widget.mWidget)
            {
            case REQ_IMAGEBUTTON:
            {
                const auto& button = icons.GetButton(widget.mImage);
                assert(std::get<Graphics::SpriteSheetIndex>(button)
                    == std::get<Graphics::SpriteSheetIndex>(icons.GetPressedButton(widget.mImage)));
                mButtons.emplace_back(
                    mLayout.GetWidgetLocation(i),
                    mLayout.GetWidgetDimensions(i),
                    std::get<Graphics::SpriteSheetIndex>(button),
                    std::get<Graphics::TextureIndex>(button),
                    std::get<Graphics::TextureIndex>(icons.GetPressedButton(widget.mImage)),
                    []{},
                    []{});
                mButtons.back().CenterImage(std::get<glm::vec2>(button));
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
        mCompass.SetHeading(heading);
    }

    void AddChildren()
    {
        for (auto& button : mButtons)
            AddChildBack(&button);

        AddChildBack(&mCompass);

        for (auto& character : mCharacters)
            AddChildBack(&character);
    }

    void UpdatePartyMembers(const BAK::GameState& gameState)
    {
        ClearChildren();

        mCharacters.clear();
        mCharacters.reserve(3);

        const auto& party = gameState.GetParty();
        mLogger.Info() << "Updating Party: " << party<< "\n";
        BAK::ActiveCharIndex person{0};
        do
        {
            const auto [spriteSheet, image, dimss] = mIcons.GetCharacterHead(
                party.GetCharacter(person).GetIndex().mValue);
            mCharacters.emplace_back(
                mLayout.GetWidgetLocation(person.mValue + sCharacterWidgetBegin),
                mLayout.GetWidgetDimensions(person.mValue + sCharacterWidgetBegin),
                spriteSheet,
                image,
                image,
                [this, character=person]{
                    ShowInventory(character);
                },
                [this, character=person]{
                    ShowPortrait(character);
                }
            );
            
            person = party.NextActiveCharacter(person);
        } while (person != BAK::ActiveCharIndex{0});

        AddChildren();
    }

    void ShowPortrait(BAK::ActiveCharIndex character)
    {
        mGuiManager.ShowCharacterPortrait(character);
    }

    void ShowInventory(BAK::ActiveCharIndex character)
    {
        mGuiManager.ShowInventory(character);
    }

private:
    IGuiManager& mGuiManager;
    const Icons& mIcons;

    BAK::Layout mLayout;

    Compass mCompass;
    std::vector<ClickButtonImage> mButtons;
    std::vector<ClickButtonImage> mCharacters;

    const Logging::Logger& mLogger;
};

}
