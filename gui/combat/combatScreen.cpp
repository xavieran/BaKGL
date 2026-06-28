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
        []{
            // Open combatant inventory...
        },
        [this]{
            if (mSelectedCharacter)
            {
                if (auto activeIdx = mGameState.GetParty().FindActiveCharacter(
                        *mSelectedCharacter))
                {
                    mGuiManager.ShowCharacterPortrait(*activeIdx);
                }
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
    mTextArea{
        glm::vec2{80, 132},
        glm::vec2{100, 88}},
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
    BAK::CharIndex character)
{
    mSelectedCharacter = character;
    UpdateActiveCharacter();
    RefreshGui();
}

void CombatScreen::DisplayMeleeInfo(BAK::MeleeInfo info)
{
    PrintMeleeInformation(info);
    RefreshGui();
}

void CombatScreen::ResetDisplay()
{
    assert(mSelectedCharacter);
    SetSelectedCharacter(*mSelectedCharacter);
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
        PrintCharacterInformation();
    }
}

void CombatScreen::PrintCharacterInformation()
{
    std::stringstream ss{};
    if (mSelectedCharacter)
    {
        auto& character = mGameState.GetParty().GetCharacter(*mSelectedCharacter);
        ss << character.GetName() << "\n";
        ss << "Health: " << character.GetSkill(BAK::SkillType::Health) << "\n";
        ss << "Stamina: " << character.GetSkill(BAK::SkillType::Stamina) << "\n";
        ss << "Speed: " << character.GetSkill(BAK::SkillType::Speed) << "\n";
        ss << "Strength: " << character.GetSkill(BAK::SkillType::Strength) << "\n";
        mTextArea.SetText(mFont, ss.str(), true);
    }
}

void CombatScreen::PrintMeleeInformation(BAK::MeleeInfo info)
{
    std::stringstream ss{};
    if (info.mSlashChance > 0 && info.mSlashDamage > 0)
    {
        ss << "Thrust        Slash\n\n";
        ss << info.mThrustDamage << "   Damage   " << info.mSlashDamage << "\n";
        ss << info.mThrustChance << "%   Accuracy   " << info.mSlashChance << "%\n";
        ss << "Left          Right\n";
    }
    else
    {
        ss << "Thrust\n\n";
        ss << info.mThrustDamage << "         Damage\n";
        ss << info.mThrustChance << "%       Accuracy\n";
        ss << "Left\n";
    }
    mTextArea.SetText(mFont, ss.str(), true);
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
            mGuiManager.GetCombatManager().DoDefend();
            break;
        case mAutoBattleButton:
            mLogger.Debug() << "AutoBattle\n";
            ExitCombat();
            break;
        case mAssessButton:
            mLogger.Debug() << "Assess\n";
            break;
        case mRestButton:
            mLogger.Debug() << "Rest\n";
            mGuiManager.GetCombatManager().DoRest();
            break;
        default:
            mLogger.Debug() << "Unhandled button: " << buttonIndex << "\n";
            break;
    }
}

void CombatScreen::ExitCombat()
{
    mGuiManager.ExitCombat(BAK::CombatResult::Won);
}

void CombatScreen::Retreat()
{
    mGuiManager.ExitCombat(BAK::CombatResult::Fled);
}

void CombatScreen::AddChildren()
{
    AddChildBack(&mFrame);

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

    AddChildBack(&mTextArea);
}
}
