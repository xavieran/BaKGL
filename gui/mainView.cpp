#include "gui/mainView.hpp"

#include "bak/gameState.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/fontManager.hpp"
#include "gui/icons.hpp"

#include <glm/glm.hpp>

#include <iostream>

namespace Gui {

MainView::MainView(
    IGuiManager& guiManager,
    const Backgrounds& backgrounds,
    const Icons& icons,
    const Font& spellFont)
:
    Widget{
        Graphics::DrawMode::Sprite,
        backgrounds.GetSpriteSheet(),
        backgrounds.GetScreen("FRAME.SCX"),
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        glm::vec2{0},
        glm::vec2{320, 200},
        true
    },
    mGuiManager{guiManager},
    mIcons{icons},
    mSpellFont{spellFont},
    mLayout{sLayoutFile},
    mActiveSpells{},
    mCompass{
        glm::vec2{144,121},
        glm::vec2{32,12},
        std::get<glm::vec2>(icons.GetCompass())
            + glm::vec2{0, 1},
        std::get<Graphics::SpriteSheetIndex>(icons.GetCompass()),
        std::get<Graphics::TextureIndex>(icons.GetCompass())
    },
    mButtons{},
    mCharacters{},
    mLogger{Logging::LogState::GetLogger("Gui::MainView")}
{
    mButtons.reserve(mLayout.GetSize());

    for (unsigned i = 0; i < mLayout.GetSize(); i++)
    {
        const auto& widget = mLayout.GetWidget(i);
        switch (widget.mWidget)
        {
        case 3: //REQ_IMAGEBUTTON
        {
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

            mButtons.back().CenterImage(std::get<glm::vec2>(button));
            // Not sure why the dims aren't right to begin with for these buttons
            if (i == sForward || i == sBackward)
            {
                mButtons.back().AdjustPosition(
                    glm::vec2{-mButtons.back().GetDimensions().x / 4 + 1.5, 0});
            }
        }
            break;
        default:
            mLogger.Info() << "Unhandled: " << i << "\n";
            break;
        }
    }

    AddChildren();
}

void MainView::SetHeading(BAK::GameHeading heading)
{
    mCompass.SetHeading(heading);
}

void MainView::HandleButton(unsigned buttonIndex)
{
    switch (buttonIndex)
    {
    case sCast:
        mGuiManager.ShowCast(false);
        break;
    case sCamp:
        mGuiManager.ShowCamp(false, nullptr);
        break;
    case sFullMap:
        mGuiManager.ShowFullMap();
        break;
    case sBookmark:
        break;
    case sMainMenu:
        mGuiManager.EnterMainMenu(true);
        break;
    default:
        break;
    }
}

void MainView::UpdatePartyMembers(const BAK::GameState& gameState)
{
    ClearChildren();

    mCharacters.clear();
    mCharacters.reserve(3);

    const auto& party = gameState.GetParty();
    mLogger.Spam() << "Updating Party: " << party<< "\n";
    BAK::ActiveCharIndex person{0};
    do
    {
        const auto [spriteSheet, image, dimss] = mIcons.GetCharacterHead(
            party.GetCharacter(person).GetIndex().mValue);
        mCharacters.emplace_back(
            mLayout.GetWidgetLocation(person.mValue + sCharacterWidgetBegin),
            mLayout.GetWidgetDimensions(person.mValue + sCharacterWidgetBegin),
            spriteSheet,
            image,
            image,
            [this, character=person]{
                ShowInventory(character);
            },
            [this, character=person]{
                ShowPortrait(character);
            }
        );
        
        person = party.NextActiveCharacter(person);
    } while (person != BAK::ActiveCharIndex{0});

    auto pos = glm::vec2{140, 1};

    // FIXME: Update these whenever time changes...
    mActiveSpells.clear();
    for (std::uint16_t i = 0; i < 6; i++)
    {
        if (gameState.GetSpellActive(BAK::StaticSpells{i}))
        {
            auto spellI = BAK::sStaticSpellMapping[i];
            mActiveSpells.emplace_back(Gui::Widget{
                Graphics::DrawMode::Sprite,
                mSpellFont.GetSpriteSheet(),
                static_cast<Graphics::TextureIndex>(
                    mSpellFont.GetFont().GetIndex(spellI)),
                Graphics::ColorMode::Texture,
                glm::vec4{1.2f, 0.f, 0.f, 1.f},
                pos,
                glm::vec2{
                    mSpellFont.GetFont().GetWidth(spellI),
                    mSpellFont.GetFont().GetHeight()},
                true
            });
            pos += glm::vec2{mSpellFont.GetFont().GetWidth(spellI) + 1, 0};
        }
    }

    AddChildren();
}

void MainView::ShowPortrait(BAK::ActiveCharIndex character)
{
    mGuiManager.ShowCharacterPortrait(character);
}

void MainView::ShowInventory(BAK::ActiveCharIndex character)
{
    mGuiManager.ShowInventory(character);
}

void MainView::AddChildren()
{
    ClearChildren();
    for (auto& button : mButtons)
    {
        AddChildBack(&button);
    }
    for (auto& spell : mActiveSpells)
    {
        AddChildBack(&spell);
    }
    AddChildBack(&mCompass);

    for (auto& character : mCharacters)
        AddChildBack(&character);
}

}
