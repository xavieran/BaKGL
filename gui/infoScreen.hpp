#pragma once

#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/actors.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/portrait.hpp"
#include "gui/widget.hpp"
#include "gui/skills.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class InfoScreen : public Widget
{
public:
    static constexpr auto sSkillRightClickDialog = BAK::KeyTarget{0x143};
    static constexpr auto sCharacterFlavourDialog = BAK::KeyTarget{0x69};

    InfoScreen(
        Graphics::SpriteManager& spriteManager,
        IGuiManager& guiManager,
        const Actors& actors,
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
        mElements{},
        mButtons{},
        mPortrait{},
        mSkills{},
        mLogger{Logging::LogState::GetLogger("Gui::InfoScreen")}
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
                "#Exit",
                [this]{ mGuiManager.ExitCharacterPortrait(); });
        }

        {
            unsigned i = 0; // portrait
            const auto& data = request.GetRequestData(i);
            int x = data.xpos + request.GetRectangle().GetXPos() + request.GetXOff();
            int y = data.ypos + request.GetRectangle().GetYPos() + request.GetYOff();
            mPortrait.emplace(
                glm::vec2{x, y},
                glm::vec2{data.width, data.height},
                actors,
                mFont,
                mSpriteSheet,
                pressedOffset + 25,
                pressedOffset + 26,
                [this](){
                    AdvanceCharacter(); },
                [this](){
                    mGameState.SetDialogContext(mSelectedCharacter);
                    mGuiManager.StartDialog(
                        sCharacterFlavourDialog, false, &mDialogScene);
                }
            );
        }

        mSkills.emplace(
            glm::vec2{15, 100},
            glm::vec2{200,200},
            mSpriteSheet,
            pressedOffset,
            request,
            [this](auto skill){
                ToggleSkill(skill);
            },
            [this](){
                mGuiManager.StartDialog(
                    sSkillRightClickDialog, false, &mDialogScene);
            });

        AddChildren();
        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
    }

    void SetSelectedCharacter(unsigned character)
    {
        mSelectedCharacter = character;
    }

    void AdvanceCharacter()
    {
        SetSelectedCharacter(
            mGameState.GetParty().NextActiveCharacter(mSelectedCharacter));
        UpdateCharacter();
    }

    void UpdateCharacter()
    {
        assert(mSkills);
        const auto& character = mGameState.GetParty().mCharacters[mSelectedCharacter];
        mSkills->UpdateSkills(mFont, character.mSkills);
        mPortrait->SetCharacter(mSelectedCharacter, character.mName);
    }

    void ToggleSkill(BAK::SkillType skill)
    {
        mLogger.Debug() << "Toggle Skill: " << BAK::ToString(skill) << "\n";
        mGameState
            .GetParty()
            .mCharacters[mSelectedCharacter]
            .mSkills.ToggleSkill(skill);
        UpdateCharacter();
    }

    void AddChildren()
    {
        for (auto& elem : mElements)
            AddChildBack(&elem);

        for (auto& but : mButtons)
            AddChildBack(&but);

        assert(mSkills);
        AddChildBack(&(*mSkills));
        assert(mPortrait);
        AddChildBack(&(*mPortrait));
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;
    unsigned mSelectedCharacter;
    Graphics::SpriteSheetIndex mSpriteSheet;
    std::vector<Widget> mElements;
    std::vector<ClickButton> mButtons;
    std::optional<Portrait> mPortrait;
    std::optional<Skills> mSkills;

    const Logging::Logger& mLogger;
};

}
