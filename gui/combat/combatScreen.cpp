#include "gui/combat/combatScreen.hpp"

#include "bak/gameState.hpp"
#include "bak/dialogSources.hpp"

#include "com/logger.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"

namespace {
constexpr auto gButtonPosOffset = glm::vec2{0, 1};
constexpr auto gButtonDimOffset = glm::vec2{1, 2};
}

namespace Gui::Combat {

CombatScreen::CombatScreen(
    IGuiManager& guiManager,
    const Backgrounds& backgrounds,
    const Icons& icons,
    const Font& font,
    BAK::GameState& gameState)
:
    // Black background
    Widget{
        RectTag{},
        glm::vec2{0, 0},
        glm::vec2{320, 200},
        glm::vec4{},
        true
    },
    mGuiManager{guiManager},
    mFont{font},
    mIcons{icons},
    mGameState{gameState},
    mDialogScene{
        []{},
        []{},
        [](const auto&){}
    },
    mLayout{sLayoutFile},
    mFrame{
        ImageTag{},
        backgrounds.GetSpriteSheet(),
        backgrounds.GetScreen(sBackground),
        glm::vec2{0},
        GetPositionInfo().mDimensions,
        true
    },
    mCharacter{
        [this]{
            if (!mSelectedCharacter)
                mSelectedCharacter = BAK::ActiveCharIndex{0};
            else
                mSelectedCharacter = mGameState.GetParty().NextActiveCharacter(*mSelectedCharacter);
            RefreshGui();
        },
        [this]{
            if (mSelectedCharacter)
            {
                mGuiManager.ShowCharacterPortrait(*mSelectedCharacter);
            }
        },
        ImageTag{},
        Graphics::SpriteSheetIndex{},
        Graphics::TextureIndex{},
        mLayout.GetWidgetLocation(mCharacterRequest) + glm::vec2{0, 7},
        mLayout.GetWidgetDimensions(mCharacterRequest) + glm::vec2{5, 0},
        true
    },
    mButtons{},
    mExit{
        mLayout.GetWidgetLocation(mExitRequest) - glm::vec2{30, 30},
        mLayout.GetWidgetDimensions(mExitRequest),
        mIcons.GetButtonTextures(mExitButton),
        [this]{
            ExitCombat();
        },
        []{}
    },
    mShoot{
        mLayout.GetWidgetLocation(mShootButton) + gButtonPosOffset,
        mLayout.GetWidgetDimensions(mShootButton) + gButtonDimOffset,
        mIcons.GetButtonTextures(mLayout.GetWidget(mShootButton).mImage),
        [this]{ HandleButton(mShootButton); },
        []{}
    },
    mCast{
        mLayout.GetWidgetLocation(mCastButton) + gButtonPosOffset,
        mLayout.GetWidgetDimensions(mCastButton) + gButtonDimOffset,
        mIcons.GetButtonTextures(mLayout.GetWidget(mCastButton).mImage),
        [this]{ HandleButton(mCastButton); },
        []{}
    },
    mSelectedCharacter{},
    mNeedRefresh{false},
    mLogger{Logging::LogState::GetLogger("Gui::CombatScreen")}
{
    mButtons.reserve(mLayout.GetSize());

    for (unsigned i = 0; i < mLayout.GetSize(); i++)
    {
        const auto& widget = mLayout.GetWidget(i);
        switch (widget.mWidget)
        {
        case 3: //REQ_IMAGEBUTTON
        {
            if (widget.mImage > 30 || i == mCharacterRequest)
            {
                continue;
            }
            if (i == mShootButton || i == mCastButton || i == mLeaveShootButton)
            {
                continue;
            }
            const auto textures = icons.GetButtonTextures(widget.mImage);
            mButtons.emplace_back(
                mLayout.GetWidgetLocation(i) + gButtonPosOffset,
                mLayout.GetWidgetDimensions(i) + gButtonDimOffset,
                textures,
                [this, buttonIndex=i]{ HandleButton(buttonIndex); },
                []{});
        } break;
        default:
            mLogger.Info() << "Unhandled: " << i << "\n";
            break;
        }
    }
}

void CombatScreen::SetSelectedCharacter(
    BAK::ActiveCharIndex character)
{
    RefreshGui();
}

void CombatScreen::StartDialog(BAK::Target target)
{
    mGuiManager.StartDialog(
        target,
        false,
        false,
        &mDialogScene);
}

/* Widget */
bool CombatScreen::OnMouseEvent(const MouseEvent& event)
{
    const bool handled = Widget::OnMouseEvent(event);
    mNeedRefresh = true;
    return handled;
}

void CombatScreen::RefreshGui()
{
    ClearChildren();

    UpdateActiveCharacter();

    AddChildren();
}

void CombatScreen::UpdateActiveCharacter()
{
    if (mSelectedCharacter)
    {
        const auto& party = mGameState.GetParty();
        const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(
            party.GetCharacter(*mSelectedCharacter).GetIndex().mValue);
        mCharacter.SetSpriteSheet(spriteSheet);
        mCharacter.SetTexture(image);
    }
}

void CombatScreen::HandleButton(unsigned buttonIndex)
{
    mLogger.Info() << "Combat button i: " << buttonIndex << "\n";
    switch (buttonIndex)
    {
        case mShootButton:
            mLogger.Debug() << "Shoot\n";
            break;
        case mCastButton:
            mLogger.Debug() << "Cast\n";
            break;
        case mRetreatButton:
            Retreat();
            break;
        case mDefendButton:
            mLogger.Debug() << "Defend\n";
            break;
        case mAutoBattleButton:
            mLogger.Debug() << "AutoBattle\n";
            break;
        case mAssessButton:
            mLogger.Debug() << "Assess\n";
            break;
        case mRestButton:
            mLogger.Debug() << "Rest\n";
            break;
        default:
            mLogger.Debug() << "Unhandled button: " << buttonIndex << "\n";
            break;
    }
}

void CombatScreen::ExitCombat()
{
    mDialogScene.SetDialogFinished([this](auto choice){
        mGuiManager.ExitCombat(BAK::CombatResult::Won);
    });
    StartDialog(BAK::DialogSources::mWonBattle);
}

void CombatScreen::Retreat()
{
    mDialogScene.SetDialogFinished([this](auto choice){
        mGuiManager.ExitCombat(BAK::CombatResult::Fled);
    });
    StartDialog(BAK::DialogSources::mRetreatSuccessful);
}

void CombatScreen::AddChildren()
{
    AddChildBack(&mFrame);

    AddChildBack(&mExit);

    AddChildBack(&mCharacter);

    if (mSelectedCharacter)
    {
        const auto& character = mGameState.GetParty().GetCharacter(*mSelectedCharacter);
        if (character.IsSwordsman()
            && character.GetInventory().FindEquipped(BAK::ItemType::Crossbow)
                != character.GetInventory().GetItems().end())
        {
            AddChildBack(&mShoot);
        }
        if (character.IsSpellcaster()
            && character.GetInventory().FindEquipped(BAK::ItemType::Staff)
                != character.GetInventory().GetItems().end())
        {
            AddChildBack(&mCast);
        }
    }

    for (auto& button : mButtons)
    {
        AddChildBack(&button);
    }
}
}
