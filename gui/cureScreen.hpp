#pragma once

#include "audio/audio.hpp"

#include "bak/dialogSources.hpp"
#include "bak/layout.hpp"
#include "bak/temple.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/info/portrait.hpp"
#include "gui/info/ratings.hpp"
#include "gui/colors.hpp"
#include "gui/teleportDest.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class CureScreen : public Widget, public IDialogScene
{
public:
    static constexpr auto sLayoutFile = "REQ_HEAL.DAT";
    static constexpr auto sInfoLayoutFile = "REQ_INFO.DAT";
    static constexpr auto sScreen = "OPTIONS1.SCX";

    static constexpr auto sCurePlayer = 0;
    static constexpr auto sNextPlayer = 1;
    static constexpr auto sDone = 2;

    static constexpr auto sPortraitWidget = 0;

    CureScreen(
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
                mGameState.SetDialogContext(character.mValue);
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

    void DisplayNPCBackground() override {}
    void DisplayPlayerBackground() override {}

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override
    {
        AdvanceCharacter();
    }

    void EnterScreen(unsigned templeNumber, unsigned cureFactor, std::function<void()>&& finished)
    {
        mFinished = finished;
        ASSERT(mFinished);
        mTempleNumber = templeNumber;
        mCureFactor = cureFactor;
        mSelectedCharacter = BAK::ActiveCharIndex{0};
        AdvanceCharacter();
    }

private:
    void CureCharacter()
    {
        if (mCost.mValue > mGameState.GetMoney().mValue)
        {
            mGuiManager.StartDialog(BAK::DialogSources::mHealDialogCantAfford, false, false, this);
        }
        else
        {
            AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{0xc});
            mGameState.SetActiveCharacter(mSelectedCharacter);
            mGuiManager.StartDialog(BAK::DialogSources::mHealDialogPostHealing, false, false, this);
            auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
            BAK::Temple::CureCharacter(character.mSkills, character.mConditions, mTempleNumber == BAK::Temple::sTempleOfSung);
        }
    }

    void AdvanceCharacter()
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

    void SetSelectedCharacter(BAK::ActiveCharIndex character)
    {
        mSelectedCharacter = character;
    }

    BAK::Royals CalculateCureCost()
    {
        auto& character = mGameState.GetParty().GetCharacter(mSelectedCharacter);
        return BAK::Temple::CalculateCureCost(
            mCureFactor,
            mTempleNumber == BAK::Temple::sTempleOfSung,
            character.mSkills,
            character.mConditions);
    }

    void UpdateCharacter()
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
        mCureText.AddText(mFont, mGameState.GetTextVariableStore()
            .SubstituteVariables(std::string{snip.GetText()}), false, false, true);
    }

    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mPortrait);
        AddChildBack(&mRatings);
        AddChildBack(&mCureText);
        AddChildBack(&mCureButton);
        AddChildBack(&mNextPlayerButton);
        AddChildBack(&mDoneButton);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;
    BAK::Layout mInfoLayout;

    BAK::ActiveCharIndex mSelectedCharacter;
    unsigned mTempleNumber;
    unsigned mCureFactor;
    std::function<void()> mFinished;
    BAK::Royals mCost;
    Portrait mPortrait;
    Ratings mRatings;
    TextBox mCureText;
    ClickButton mCureButton;
    ClickButton mNextPlayerButton;
    ClickButton mDoneButton;

    const Logging::Logger& mLogger;
};

}
