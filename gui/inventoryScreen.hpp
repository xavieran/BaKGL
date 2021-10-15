#pragma once

#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
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
    static constexpr auto sLayoutFile = "REQ_INV.DAT";
    static constexpr auto sBackground = "INVENTOR.SCX";

    InventoryScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState)
    :
        // Black background
        Widget{
            RectTag{},
            glm::vec2{0},
            glm::vec2{320, 200},
            Color::black,
            true
        },
        mGuiManager{guiManager},
        mFont{font},
        mIcons{icons},
        mGameState{gameState},
        mDialogScene{},
        mLayout{sLayoutFile},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            GetPositionInfo().mPosition,
            GetPositionInfo().mDimensions,
            true
        },
        mCharacterLocations{},
        mCharacters{},
        mExit{
            GetRequestLocation(5),
            GetRequestDims(5),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(13)),
            std::get<Graphics::TextureIndex>(mIcons.GetButton(13)),
            std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(13)),
            [this]{ mGuiManager.ExitInventory(); },
            []{}
        },
        mSelectedCharacter{0},
        mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
    {
        // Character heads
        for (unsigned i = 0; i < 3; i++)
        {
            mCharacterLocations.emplace_back(
                GetRequestLocation(i),
                GetRequestDims(i));
        }

        AddChildren();
    }

    glm::vec2 GetRequestLocation(unsigned i)
    {
        const auto data = mLayout.GetRequestData(i);
        int x = data.xpos + mLayout.GetRectangle().GetXPos() + mLayout.GetXOff();
        int y = data.ypos + mLayout.GetRectangle().GetYPos() + mLayout.GetYOff();
        return glm::vec2{x, y};
    }

    glm::vec2 GetRequestDims(unsigned i)
    {
        const auto data = mLayout.GetRequestData(i);
        return glm::vec2{data.width, data.height};
    }


    void UpdatePartyMembers()
    {
        ClearChildren();

        mCharacters.clear();
        mCharacters.reserve(mCharacterLocations.size());

        const auto& party = mGameState.GetParty();
        mLogger.Info() << "Updating Party: " << party<< "\n";
        for (unsigned person = 0; person < party.mActiveCharacters.size(); person++)
        {
            assert(person < mCharacterLocations.size());
            const auto& [pos, dims] = mCharacterLocations[person];
            const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(party.mActiveCharacters[person]);
            mCharacters.emplace_back(
                pos,
                dims,
                spriteSheet,
                image,
                image,
                [character=party.mActiveCharacters[person]]{
                    // Switch character
                },
                [this, character=party.mActiveCharacters[person]]{
                    mGuiManager.ShowCharacterPortrait(character);
                }
            );
        }

        AddChildren();
    }

    void SetSelectedCharacter(unsigned character)
    {
        mSelectedCharacter = character;
        UpdatePartyMembers();
    }

    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);
        AddChildBack(&mExit);

        for (auto& character : mCharacters)
            AddChildBack(&character);

    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;

    RequestResource mLayout;

    Widget mFrame;

    std::vector<std::pair<glm::vec2, glm::vec2>> mCharacterLocations;
    std::vector<ClickButtonImage> mCharacters;
    ClickButtonImage mExit;

    //Widget mOtherInventory; // e.g. keys/shop/bag/corpse, etc...

    unsigned mSelectedCharacter;
    const Logging::Logger& mLogger;
};

}
