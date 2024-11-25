#include "gui/temple/cureScreen.hpp"

#include "audio/audio.hpp"

#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/money.hpp"
#include "bak/layout.hpp"
#include "bak/temple.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/info/portrait.hpp"
#include "gui/info/ratings.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

CureScreen::CureScreen(
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
        backgrounds.GetScreen(sScreen),
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        glm::vec2{0},
        glm::vec2{320, 200},
        true
    },
    mGuiManager{guiManager},
    mFont{font},
    mGameState{gameState},
    mIcons{icons},
    mLayout{sLayoutFile},
    mInfoLayout{sInfoLayoutFile},
    mSelectedCharacter{BAK::ActiveCharIndex{0}},
    mTempleNumber{BAK::Temple::sTempleOfSung},
    mCureFactor{65},
    mFinished{nullptr},
    mCost{BAK::Royals{0}},
    mPortrait{
        mInfoLayout.GetWidgetLocation(sPortraitWidget),
        mInfoLayout.GetWidgetDimensions(sPortraitWidget),
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
                BAK::DialogSources::mCharacterFlavourDialog, false, false, this);
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
    mCureText{
        glm::vec2{34, 94},
        glm::vec2{240, 80}
    },
    mCureButton{
        mLayout.GetWidgetLocation(sCurePlayer),
        mLayout.GetWidgetDimensions(sCurePlayer) + glm::vec2{0, 1},
        mFont,
        "#Cure Player",
        [this]{ 
            CureCharacter();
        }
    },
    mNextPlayerButton{
        mLayout.GetWidgetLocation(sNextPlayer),
        mLayout.GetWidgetDimensions(sNextPlayer) + glm::vec2{0, 1},
        mFont,
        "#Next Player",
        [this]{ 
            AdvanceCharacter();
        }
    },
    mDoneButton{
        mLayout.GetWidgetLocation(sDone),
        mLayout.GetWidgetDimensions(sDone) + glm::vec2{0, 1},
        mFont,
        "#Done",
        [this]{ 
            mGuiManager.ExitSimpleScreen();
            ASSERT(mFinished);
            mFinished();
        }
    },
    mLogger{Logging::LogState::GetLogger("Gui::CureScreen")}
{
    AddChildren();
}

void CureScreen::DisplayNPCBackground() {}
void CureScreen::DisplayPlayerBackground() {}

void CureScreen::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    AdvanceCharacter();
}

void CureScreen::EnterScreen(unsigned templeNumber, unsigned cureFactor, std::function<void()>&& finished)
{
    mFinished = finished;
    ASSERT(mFinished);
    mTempleNumber = templeNumber;
    mCureFactor = cureFactor;
    mSelectedCharacter = BAK::ActiveCharIndex{0};
    AdvanceCharacter();
}

void CureScreen::CureCharacter()
{
    if (mCost.mValue > mGameState.GetMoney().mValue)
    {
        mGuiManager.StartDialog(BAK::DialogSources::mHealDialogCantAfford, false, false, this);
    }
    else
    {
        AudioA::GetAudioManager().PlaySound(AudioA::SoundIndex{0xc});
        mGameState.SetActiveCharacter(mSelectedCharacter);
        mGuiManager.StartDialog(BAK::DialogSources::mHealDialogPostHealing, false, false, this);
        auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
        BAK::Temple::CureCharacter(character.mSkills, character.mConditions, mTempleNumber == BAK::Temple::sTempleOfSung);
    }
}

void CureScreen::AdvanceCharacter()
{
    const auto startCharacter = mSelectedCharacter;
    SetSelectedCharacter(
        mGameState.GetParty().NextActiveCharacter(mSelectedCharacter));
    while (CalculateCureCost().mValue == 0
        && mSelectedCharacter != startCharacter)
    {
        SetSelectedCharacter(
            mGameState.GetParty().NextActiveCharacter(mSelectedCharacter));
    }

    if (mSelectedCharacter == startCharacter && CalculateCureCost().mValue == 0)
    {
        mGuiManager.ExitSimpleScreen();
        mFinished();
    }

    UpdateCharacter();
}

void CureScreen::SetSelectedCharacter(BAK::ActiveCharIndex character)
{
    mSelectedCharacter = character;
}

BAK::Royals CureScreen::CalculateCureCost()
{
    auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
    return BAK::Temple::CalculateCureCost(
        mCureFactor,
        mTempleNumber == BAK::Temple::sTempleOfSung,
        character.mSkills,
        character.mConditions,
        character.mSkillAffectors);
}

void CureScreen::UpdateCharacter()
{
    auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
    mPortrait.SetCharacter(character.GetIndex(), character.mName);
    mRatings.SetCharacter(character);
    mCost = CalculateCureCost();
    mGameState.SetItemValue(mCost);

    // FIXME: This is awful... would be nice to generically deal with text vars
    const auto snip = BAK::DialogStore::Get().GetSnippet(BAK::DialogSources::mHealDialogCost);
    mGameState.SetActiveCharacter(mSelectedCharacter);
    mGameState.SetCharacterTextVariables();
    // For some reason the dialog action sets text variable 1 for cost but the dialog uses 0 for cost.
    mGameState.GetTextVariableStore().SetTextVariable(0, BAK::ToShopDialogString(mCost));
    mCureText.SetText(mFont, mGameState.GetTextVariableStore()
        .SubstituteVariables(std::string{snip.GetText()}), false, false, true);
}

void CureScreen::AddChildren()
{
    ClearChildren();

    AddChildBack(&mPortrait);
    AddChildBack(&mRatings);
    AddChildBack(&mCureText);
    AddChildBack(&mCureButton);
    AddChildBack(&mNextPlayerButton);
    AddChildBack(&mDoneButton);
}

}
