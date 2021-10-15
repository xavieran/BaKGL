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

    // Request offsets
    static constexpr auto mContainerTypeRequest = 3;
    static constexpr auto mUseItemRequest = 4;
    static constexpr auto mExitRequest = 5;
    static constexpr auto mExitButton = 13;
    static constexpr auto mGoldRequest = 6;

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
        mCharacters{},
        mExit{
            GetRequestLocation(mExitRequest),
            GetRequestDims(mExitRequest),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(mExitButton)),
            std::get<Graphics::TextureIndex>(mIcons.GetButton(mExitButton)),
            std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(mExitButton)),
            [this]{ mGuiManager.ExitInventory(); },
            []{}
        },
        mGoldDisplay{
            GetRequestLocation(mGoldRequest),
            GetRequestDims(mGoldRequest)
        },
        mContainerTypeDisplay{
            GetRequestLocation(mContainerTypeRequest),
            GetRequestDims(mContainerTypeRequest),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            []{}, // Goto Keys, or goto Shop, or Goto Bag, or Goto Container...
            []{}
        },
        mSelectedCharacter{0},
        mLogger{Logging::LogState::GetLogger("Gui::InventoryScreen")}
    {
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
        mCharacters.reserve(3);

        const auto& party = mGameState.GetParty();
        mLogger.Info() << "Updating Party: " << party<< "\n";
        for (unsigned person = 0; person < party.mActiveCharacters.size(); person++)
        {
            const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(party.mActiveCharacters[person]);
            mCharacters.emplace_back(
                GetRequestLocation(person),
                GetRequestDims(person),
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
        UpdateGold();
    }

    void UpdateGold()
    {
        const auto gold = mGameState.GetParty().GetGold();
        const auto sovereigns = BAK::GetSovereigns(gold);
        const auto royals = BAK::GetRemainingRoyals(gold);
        std::stringstream ss{};
        ss << "#" << sovereigns << "s " << royals << "r";
        const auto [dims, _] = mGoldDisplay.AddText(mFont, ss.str());

        // Justify text to the right
        const auto basePos = GetRequestLocation(mGoldRequest);
        const auto newPos = basePos 
            + glm::vec2{
                3 + GetRequestDims(mGoldRequest).x - dims.x,
                4};

        mGoldDisplay.SetPosition(newPos);
    }

    void SetContainerTypeImage()
    {
        const auto [ss, ti, dims] = mIcons.GetInventoryMiscIcon(11);
        mContainerTypeDisplay.CenterImage(dims);
    }

    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);
        AddChildBack(&mExit);
        AddChildBack(&mGoldDisplay);

        SetContainerTypeImage();
        AddChildBack(&mContainerTypeDisplay);

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

    std::vector<ClickButtonImage> mCharacters;
    ClickButtonImage mExit;
    TextBox mGoldDisplay;
    // click into shop or keys, etc.
    ClickButtonImage mContainerTypeDisplay;


    unsigned mSelectedCharacter;
    const Logging::Logger& mLogger;
};

}
