#include "gui/combat/combatScreen.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"

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
    mCharacters{},
    mButtons{},
    mExit{
        mLayout.GetWidgetLocation(mExitRequest) - glm::vec2{30, 30},
        mLayout.GetWidgetDimensions(mExitRequest),
        std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(mExitButton)),
        std::get<Graphics::TextureIndex>(mIcons.GetButton(mExitButton)),
        std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(mExitButton)),
        [this]{
            mGuiManager.ExitSimpleScreen();
        },
        []{}
    },
    mSelectedCharacter{},
    mNeedRefresh{false},
    mLogger{Logging::LogState::GetLogger("Gui::CombatScreen")}
{
    mCharacters.reserve(3);
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
            const auto& button = icons.GetButton(widget.mImage);
            assert(std::get<Graphics::SpriteSheetIndex>(button)
                == std::get<Graphics::SpriteSheetIndex>(icons.GetPressedButton(widget.mImage)));
            mButtons.emplace_back(
                mLayout.GetWidgetLocation(i),
                mLayout.GetWidgetDimensions(i),
                std::get<Graphics::SpriteSheetIndex>(button),
                std::get<Graphics::TextureIndex>(button),
                std::get<Graphics::TextureIndex>(icons.GetPressedButton(widget.mImage)),
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

    UpdatePartyMembers();

    AddChildren();
}

void CombatScreen::UpdatePartyMembers()
{
    mCharacters.clear();

    const auto& party = mGameState.GetParty();
    BAK::ActiveCharIndex person{0};
    do
    {
        const auto [spriteSheet, image, _] = mIcons.GetCharacterHead(
            party.GetCharacter(person).GetIndex().mValue);
        mCharacters.emplace_back(
            [this, character=person]{
                // Switch character
                SetSelectedCharacter(character);
            },
            [this, character=person]{
                mGuiManager.ShowCharacterPortrait(character);
            },
            ImageTag{},
            spriteSheet,
            image,
            mLayout.GetWidgetLocation(mCharacterRequest),
            mLayout.GetWidgetDimensions(mCharacterRequest),
            true
        );

        if (person != mSelectedCharacter)
        {
            mCharacters[person.mValue].SetColor(glm::vec4{.05, .05, .05, 1}); 
            mCharacters[person.mValue].SetColorMode(Graphics::ColorMode::TintColor);
        }

        person = party.NextActiveCharacter(person);
    } while (person != BAK::ActiveCharIndex{0});
}

void CombatScreen::HandleButton(unsigned buttonIndex)
{
    mLogger.Info() << "Combat button i: " << buttonIndex << "\n";
    switch (buttonIndex)
    {
        case mCastButton:
            mLogger.Debug() << "Cast\n";
            break;
        case mRetreatButton:
            mLogger.Debug() << "Retreat\n";
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

void CombatScreen::AddChildren()
{
    AddChildBack(&mFrame);

    AddChildBack(&mExit);

    for (auto& character : mCharacters)
    {
        AddChildBack(&character);
    }

    for (auto& button : mButtons)
    {
        AddChildBack(&button);
    }
}

}
