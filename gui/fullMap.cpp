#include "gui/fullMap.hpp"

#include "bak/coordinates.hpp"
#include "bak/dialogSources.hpp"
#include "bak/fmap.hpp"
#include "bak/gameState.hpp"
#include "bak/layout.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/callbackDelay.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/tickAnimator.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace Gui {

FullMap::FullMap(
    IGuiManager& guiManager,
    const Backgrounds& backgrounds,
    const Icons& icons,
    const Font& font,
    BAK::GameState& gameState)
:
    Widget{
        Graphics::DrawMode::Sprite,
        backgrounds.GetSpriteSheet(),
        backgrounds.GetScreen("FULLMAP.SCX"),
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        glm::vec2{0},
        glm::vec2{320, 200}, true
    },
    mGuiManager{guiManager},
    mFont{font},
    mGameState{gameState},
    mIcons{icons},
    mFMapXY{},
    mFMapTowns{},
    mLayout{sLayoutFile},
    mExitButton{
        mLayout.GetWidgetLocation(sExitWidget),
        mLayout.GetWidgetDimensions(sExitWidget),
        mFont,
        "#Exit",
        [this]{ 
            mGuiManager.DoFade(.8, [this]{
                mPlayerPositionFlasher->Stop();
                mGuiManager.ExitSimpleScreen(); });
        }
    },
    mPopup{
        glm::vec2{},
        glm::vec2{},
        Color::buttonBackground,
        Color::buttonHighlight,
        Color::buttonShadow,
        Color::black
    },
    mPopupText{
        glm::vec2{},
        glm::vec2{}
    },
    mPlayerLocation{
        ImageTag{},
        std::get<Graphics::SpriteSheetIndex>(icons.GetFullMapIcon(0)),
        std::get<Graphics::TextureIndex>(icons.GetFullMapIcon(0)),
        mFMapXY.GetTileCoords(BAK::ZoneNumber{1}, glm::uvec2{10, 15}),
        std::get<glm::vec2>(icons.GetFullMapIcon(0)),
        false
    },  
    mTowns{},
    mGameStartScreenMode{false},
    mLogger{Logging::LogState::GetLogger("Gui::FullMap")}
{
    mTowns.reserve(mFMapTowns.GetTowns().size());
    for (const auto& town : mFMapTowns.GetTowns())
    {
        mTowns.emplace_back(
            town.mCoord,
            glm::vec2{5, 5},
            mFont,
            town.mName);
    }

    mPopup.AddChildBack(&mPopupText);
}

void FullMap::DisplayMapMode()
{
    mGameStartScreenMode = false;
    StartPlayerPositionFlasher();
    AddChildren();
}

void FullMap::DisplayGameStartMode(BAK::Chapter chapter, BAK::MapLocation location)
{
    mGameStartScreenMode = true;
    SetPlayerLocation(location);
    DisplayGameStart(chapter);
    AddChildren();

    StartPlayerPositionFlasher();

    mGuiManager.AddAnimator(
        std::make_unique<CallbackDelay>(
            [&](){
                mGuiManager.EnterMainView();
                mPlayerPositionFlasher->Stop();
            },
            3));
}

void FullMap::UpdateLocation()
{
    SetPlayerLocation(mGameState.GetZone(), mGameState.GetLocation());
}

void FullMap::SetPlayerLocation(
    BAK::ZoneNumber zone,
    BAK::GamePositionAndHeading location)
{
    mPlayerPositionBaseIcon = BAK::HeadingToFullMapAngle(location.mHeading);

    UpdatePlayerPositionIcon();

    mPlayerLocation.SetCenter(
        mFMapXY.GetTileCoords(
            zone, 
            BAK::GetTile(location.mPosition)));
}

void FullMap::SetPlayerLocation(BAK::MapLocation location)
{
    mPlayerPositionBaseIcon = BAK::HeadingToFullMapAngle(location.mHeading);
    mPlayerLocation.SetCenter(location.mPosition);
    UpdatePlayerPositionIcon();
}

void FullMap::DisplayGameStart(BAK::Chapter chapter)
{
    const auto& snippet = BAK::DialogStore::Get().GetSnippet(
        BAK::DialogSources::GetChapterStartText(chapter));
    assert(snippet.GetPopup());
    const auto popup = snippet.GetPopup();
    mLogger.Debug() << "Show snippet;" << snippet << "\n";
    mPopup.SetPosition(popup->mPos);
    mPopup.SetDimensions(popup->mDims);
    mPopupText.SetPosition(glm::vec2{1});
    mPopupText.SetDimensions(popup->mDims);
    mPopupText.SetText(mFont, snippet.GetText(), true, true);
}

void FullMap::StartPlayerPositionFlasher()
{
    auto flasher = std::make_unique<TickAnimator>(
        .1,
        [&](){
            if (mPlayerPositionIconOffset == 3)
            {
                mPlayerPositionIconPulseDirection = -1;
            }
            else if (mPlayerPositionIconOffset == 0)
            {
                mPlayerPositionIconPulseDirection = 1;
            }

            mPlayerPositionIconOffset += mPlayerPositionIconPulseDirection;
            UpdatePlayerPositionIcon();
        });
    mPlayerPositionFlasher = flasher.get();
    mGuiManager.AddAnimator(std::move(flasher));
}

void FullMap::UpdatePlayerPositionIcon()
{
    const auto& [ss, ti, dims] = mIcons.GetFullMapIcon(mPlayerPositionBaseIcon + mPlayerPositionIconOffset);
    mPlayerLocation.SetSpriteSheet(ss);
    mPlayerLocation.SetTexture(ti);
    mPlayerLocation.SetDimensions(dims);
}

void FullMap::AddChildren()
{
    ClearChildren();

    if (mGameStartScreenMode)
    {
        AddChildBack(&mPopup);
    }
    else
    {
        AddChildBack(&mExitButton);
        for (auto& t : mTowns)
            AddChildBack(&t);
    }

    AddChildBack(&mPlayerLocation);
}

}

