#pragma once

#include "bak/layout.hpp"
#include "bak/textureFactory.hpp"

#include "gui/info/portrait.hpp"
#include "gui/info/ratings.hpp"
#include "gui/info/skills.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/widget.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class InfoScreen : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_INFO.DAT";
    static constexpr auto sSkillRightClickDialog = BAK::KeyTarget{0x143};
    static constexpr auto sCharacterFlavourDialog = BAK::KeyTarget{0x69};

    static constexpr auto sPortraitWidget = 0;
    static constexpr auto sExitWidget = 1;

    InfoScreen(
        IGuiManager& guiManager,
        const Actors& actors,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("OPTIONS1.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mGuiManager{guiManager},
        mFont{font},
        mGameState{gameState},
        mDialogScene{},
        mSelectedCharacter{0},
        mLayout{sLayoutFile},
        mExitButton{
            mLayout.GetWidgetLocation(sExitWidget),
            mLayout.GetWidgetDimensions(sExitWidget),
            mFont,
            "#Exit",
            [this]{ mGuiManager.DoFade(0.8, [this]{ mGuiManager.ExitSimpleScreen(); }); }
        },
        mPortrait{
            mLayout.GetWidgetLocation(sPortraitWidget),
            mLayout.GetWidgetDimensions(sPortraitWidget),
            actors,
            mFont,
            std::get<Graphics::SpriteSheetIndex>(icons.GetStippledBorderVertical()),
            std::get<Graphics::TextureIndex>(icons.GetStippledBorderHorizontal()),
            std::get<Graphics::TextureIndex>(icons.GetStippledBorderVertical()),
            [this](){
                AdvanceCharacter(); },
            [this](){
                const auto character = mGameState
                    .GetParty()
                    .GetCharacter(BAK::ActiveCharIndex{mSelectedCharacter})
                    .GetIndex();

                mGameState.SetDialogContext(character.mValue);
                mGuiManager.StartDialog(
                    sCharacterFlavourDialog, false, false, &mDialogScene);
            }
        },
        mRatings{
            mPortrait.GetPositionInfo().mPosition 
                + glm::vec2{mPortrait.GetPositionInfo().mDimensions.x + 4, 0},
            glm::vec2{222, mPortrait.GetPositionInfo().mDimensions.y},
            mFont,
            std::get<Graphics::SpriteSheetIndex>(icons.GetStippledBorderVertical()),
            std::get<Graphics::TextureIndex>(icons.GetStippledBorderHorizontal()),
            std::get<Graphics::TextureIndex>(icons.GetStippledBorderVertical())
        },
        mSkills{
            glm::vec2{15, 100},
            glm::vec2{200,200},
            std::get<Graphics::SpriteSheetIndex>(icons.GetInventoryIcon(120)),
            std::get<Graphics::TextureIndex>(icons.GetInventoryIcon(120)),
            mLayout,
            [this](auto skill){
                ToggleSkill(skill);
            },
            [this](){
                mGuiManager.StartDialog(
                    sSkillRightClickDialog, false, false, &mDialogScene);
            }
        },
        mLogger{Logging::LogState::GetLogger("Gui::InfoScreen")}
    {
        AddChildren();
    }

    void SetSelectedCharacter(BAK::ActiveCharIndex character)
    {
        mSelectedCharacter = character;
        mGameState.GetParty().GetCharacter(character).UpdateSkills();
    }

    void AdvanceCharacter()
    {
        SetSelectedCharacter(
            mGameState.GetParty().NextActiveCharacter(mSelectedCharacter));
        UpdateCharacter();
    }

    void UpdateCharacter()
    {
        auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
        mSkills.UpdateSkills(mFont, character.mSkills);
        mPortrait.SetCharacter(character.GetIndex(), character.mName);
        mRatings.SetCharacter(character.mSkills, character.mConditions);
        character.mSkills.ClearUnseenImprovements();
    }

    void ToggleSkill(BAK::SkillType skill)
    {
        mLogger.Debug() << "Toggle Skill: " << BAK::ToString(skill) << "\n";
        mGameState
            .GetParty()
            .GetCharacter(mSelectedCharacter)
            .mSkills.ToggleSkill(skill);
        UpdateCharacter();
    }

    void AddChildren()
    {
        AddChildBack(&mExitButton);
        AddChildBack(&mSkills);
        AddChildBack(&mPortrait);
        AddChildBack(&mRatings);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;
    BAK::ActiveCharIndex mSelectedCharacter;

    BAK::Layout mLayout;

    ClickButton mExitButton;
    Portrait mPortrait;
    Ratings mRatings;
    Skills mSkills;

    const Logging::Logger& mLogger;
};

}
