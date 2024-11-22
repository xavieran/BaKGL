#include "gui/info/infoScreen.hpp"

#include "bak/dialogSources.hpp"
#include "bak/layout.hpp"

#include "bak/gameState.hpp"

#include "gui/info/portrait.hpp"
#include "gui/info/ratings.hpp"
#include "gui/info/skills.hpp"
#include "gui/info/spells.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/clickButton.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

InfoScreen::InfoScreen(
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
        [this]{ mGuiManager.DoFade(0.5, [this]{ mGuiManager.ExitSimpleScreen(); }); }
    },
    mSpellsButton{
        mLayout.GetWidgetLocation(sSpellsWidget),
        mLayout.GetWidgetDimensions(sSpellsWidget),
        mFont,
        "#Spells",
        [this]{ mGuiManager.DoFade(0.5, [this]{ 
            mSpells.SetSelectedCharacter(mSelectedCharacter);
            mGuiManager.GetScreenStack().PushScreen(&mSpells); }); },
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

            mGameState.SetDialogContext_7530(character.mValue);
            mGuiManager.StartDialog(
                BAK::DialogSources::mCharacterFlavourDialog, false, false, &mDialogScene);
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
    mSpells{
        mGuiManager,
        backgrounds,
        icons,
        font,
        gameState
    }
{
    AddChildren();
}

void InfoScreen::SetSelectedCharacter(BAK::ActiveCharIndex character)
{
    mSelectedCharacter = character;
    mGameState.GetParty().GetCharacter(character).UpdateSkills();
    AddChildren();
}

void InfoScreen::UpdateCharacter()
{
    auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
    mSkills.UpdateSkills(mFont, character.mSkills);
    mPortrait.SetCharacter(character.GetIndex(), character.mName);
    mRatings.SetCharacter(character);
    character.mSkills.ClearUnseenImprovements();
    AddChildren();
}

void InfoScreen::AdvanceCharacter()
{
    SetSelectedCharacter(
        mGameState.GetParty().NextActiveCharacter(mSelectedCharacter));
    UpdateCharacter();
}

void InfoScreen::ToggleSkill(BAK::SkillType skill)
{
    mGameState
        .GetParty()
        .GetCharacter(mSelectedCharacter)
        .mSkills.ToggleSkill(skill);
    UpdateCharacter();
}

void InfoScreen::AddChildren()
{
    ClearChildren();
    AddChildBack(&mExitButton);
    AddChildBack(&mSkills);
    AddChildBack(&mPortrait);
    AddChildBack(&mRatings);

    if (mGameState.GetParty().GetCharacter(mSelectedCharacter).IsSpellcaster())
    {
        AddChildBack(&mSpellsButton);
    }
}
}
