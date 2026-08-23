#include "gui/mainView.hpp"

#include "bak/dialog.hpp"
#include "bak/dialogSources.hpp"
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
    const Font& spellFont,
    const Font& gameFont)
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
    mGameFont{gameFont},
    mLayout{sLayoutFile},
    mMapLayout{sMapLayoutFile},
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
    mBookmarkPopup{
        glm::vec2{60, 35},
        glm::vec2{200, 55},
        mGameFont,
        "",
        []{}
    },
    mLogger{Logging::LogState::GetLogger("Gui::MainView")}
{
    const auto& snippet = BAK::DialogStore::Get().GetSnippet(
        BAK::DialogSources::mBookmarkCheck);
    const auto popup = snippet.GetPopup();
    assert(popup);
    mBookmarkPopup.SetPosition(popup->mPos);
    mBookmarkPopup.SetDimensions(popup->mDims);
    mBookmarkPopup.SetText(snippet.GetText(), true);
    mBookmarkPopup.SetInactive();

    mButtons.reserve(mLayout.GetSize());
    mMapButtons.reserve(mMapLayout.GetSize());

    auto AddButton = [&](auto index, const auto& widget, auto& buttons) 
    {
        if (index == sFollowRoad)
        {
            mFollowRoadOnImage = widget.mImage;
            mFollowRoadOffImage = widget.mImage + 1;
        }

        switch (widget.mWidget)
        {
        case 3: //REQ_IMAGEBUTTON
        case 4: //REQ_IMAGEBUTTON
        {
            const auto textures = icons.GetButtonTextures(widget.mImage);
            const auto& button = icons.GetButton(widget.mImage);
            assert(std::get<Graphics::SpriteSheetIndex>(button) == textures.mSpriteSheet);
            auto locationAdjustment = (index == sFollowRoad) ? glm::vec2{-.5, .5} : glm::vec2{};
            buttons.emplace_back(
                mLayout.GetWidgetLocation(index) + locationAdjustment,
                mLayout.GetWidgetDimensions(index),
                textures,
                [this, buttonIndex=index]{ HandleButton(buttonIndex); },
                []{});

            buttons.back().CenterImage(std::get<glm::vec2>(button));
            // Not sure why the dims aren't right to begin with for these buttons
            if (index == sForward || index == sBackward)
            {
                buttons.back().AdjustPosition(
                    glm::vec2{-buttons.back().GetDimensions().x / 4 + 1.5, 0});
            }
        }
            break;
        default:
            mLogger.Info() << "Unhandled: " << index << "\n";
            break;
        }
    };

    for (unsigned i = 0; i < mLayout.GetSize(); i++)
    {
        const auto& widget = mLayout.GetWidget(i);
        AddButton(i, widget, mButtons);
    }

    for (unsigned i = 0; i < mMapLayout.GetSize(); i++)
    {
        const auto& widget = mMapLayout.GetWidget(i);
        AddButton(i, widget, mMapButtons);
    }
    AddChildren();
}

void MainView::SetHeading(BAK::GameHeading heading)
{
    mCompass.SetHeading(heading);
}

void MainView::HandleMapView()
{
    mLogger.Info() << __FUNCTION__ << "\n";
    if (mIsInMapView)
    {
        mGuiManager.ShowFullMap();
    }
    else
    {
        mGuiManager.DoFade(1.0, [&]{
            mGuiManager.GetCameraManager().ShowOverheadView();
            mIsInMapView = true;
            AddChildren();
        });
    }
}

void MainView::HandleExit()
{
    if (mIsInMapView)
    {
        mGuiManager.DoFade(1.0, [&]{
            mGuiManager.GetCameraManager().ShowFirstPersonView();
            mIsInMapView = false;
            AddChildren();
        });
    }
    else
    {
        mGuiManager.EnterMainMenu(true);
    }
}

void MainView::HandleCast()
{
    if (mIsInMapView)
    {
        mGuiManager.GetCameraManager().ZoomOut();
    }
    else
    {
        mGuiManager.ShowCast(false);
    }
}

void MainView::HandleBookmark()
{
    if (mIsInMapView)
    {
        mGuiManager.GetCameraManager().ZoomIn();
    }
    else
    {
        mShowingBookmarkDialog = true;
        mNeedRefresh = true;
    }
}

void MainView::HandleButton(unsigned buttonIndex)
{
    switch (buttonIndex)
    {
    case sCast:
        HandleCast();
        break;
    case sFollowRoad:
        mGuiManager.GetCameraManager().ToggleFollowRoad();
        break;
    case sCamp:
        mGuiManager.ShowCamp(false, nullptr);
        break;
    case sFullMap:
        HandleMapView();
        break;
    case sBookmark:
        HandleBookmark();
        break;
    case sMainMenu:
        HandleExit();
        break;
    default:
        break;
    }
}

void MainView::SetCanSaveBookmark(bool canSaveBookmark)
{
    mCanSaveBookmark = canSaveBookmark;
    mNeedRefresh = true;
}

void MainView::SetFollowRoadVisible(bool visible)
{
    if (visible != mFollowRoadButtonVisible)
    {
        mFollowRoadButtonVisible = visible;
        AddChildren();
    }
}

void MainView::SetFollowRoadActive(bool active)
{
    const auto textures = mIcons.GetButtonTextures(
        active ? mFollowRoadOnImage : mFollowRoadOffImage);
    mButtons[sFollowRoad].SetTexture(
        textures.mSpriteSheet,
        textures.mNormal);
    mButtons[sFollowRoad].CenterImage(
        mLayout.GetWidgetDimensions(sFollowRoad) - glm::vec2{1});
    mMapButtons[sFollowRoad].SetTexture(
        textures.mSpriteSheet,
        textures.mNormal);
    mMapButtons[sFollowRoad].CenterImage(
        mLayout.GetWidgetDimensions(sFollowRoad) - glm::vec2{1});

}

void MainView::SetZoomOutVisible(bool visible)
{
    mZoomOutVisible = visible;
    AddChildren();
}

void MainView::SetZoomInVisible(bool visible)
{
    mZoomInVisible = visible;
    AddChildren();
}

bool MainView::OnMouseEvent(const MouseEvent& event)
{
    if (mShowingBookmarkDialog)
    {
        if (std::holds_alternative<LeftMousePress>(event))
        {
            mGuiManager.SaveBookmark();
        }

        if (std::holds_alternative<LeftMousePress>(event)
            || std::holds_alternative<RightMousePress>(event))
        {
            mShowingBookmarkDialog = false;
            mNeedRefresh = true;
        }

        if (mNeedRefresh)
        {
            AddChildren();
            mNeedRefresh = false;
        }

        return true;
    }

    const bool handled = Widget::OnMouseEvent(event);

    if (mNeedRefresh)
    {
        AddChildren();
        mNeedRefresh = false;
    }

    return handled;
}

void MainView::UpdatePartyMembers(const BAK::GameState& gameState)
{
    ClearChildren();

    mCharacters.clear();
    mCharacters.reserve(3);
    if (!gameState.IsGameLoaded())
    {
        return;
    }

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
    if (mIsInMapView)
    {
        for (unsigned i = 0; i < mMapButtons.size(); i++)
        {
            if (i == sFollowRoad && !mFollowRoadButtonVisible)
            {
                continue;
            }
            if (i == sCast && !mZoomOutVisible)
            {
                continue;
            }
            if (i == sBookmark && !mZoomInVisible)
            {
                continue;
            }
            AddChildBack(&mMapButtons[i]);
        }
    }
    else
    {
        for (unsigned i = 0; i < mButtons.size(); i++)
        {
            if (i == sBookmark && !mCanSaveBookmark)
            {
                continue;
            }
            if (i == sFollowRoad && !mFollowRoadButtonVisible)
            {
                continue;
            }
            AddChildBack(&mButtons[i]);
        }
    }
    
    for (auto& spell : mActiveSpells)
    {
        AddChildBack(&spell);
    }
    AddChildBack(&mCompass);

    for (auto& character : mCharacters)
    {
        AddChildBack(&character);
    }

    if (mShowingBookmarkDialog)
    {
        AddChildBack(&mBookmarkPopup);
    }
}

}
