#include "gui/combat/combatScreen.hpp"

#include "bak/gameState.hpp"

#include "com/logger.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
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
    mExit{
        mLayout.GetWidgetLocation(mExitRequest),
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

void CombatScreen::AddChildren()
{
    AddChildBack(&mFrame);

    AddChildBack(&mExit);

    for (auto& character : mCharacters)
    {
        AddChildBack(&character);
    }
}

}
