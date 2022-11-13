#pragma once

#include "bak/coordinates.hpp"
#include "bak/fmap.hpp"
#include "bak/layout.hpp"
#include "bak/textureFactory.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/townLabel.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class FullMap : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_FMAP.DAT";

    static constexpr auto sExitWidget = 0;

    FullMap(
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
        mDialogScene{},
        mFMapXY{},
        mFMapTowns{},
        mLayout{sLayoutFile},
        mExitButton{
            mLayout.GetWidgetLocation(sExitWidget),
            mLayout.GetWidgetDimensions(sExitWidget),
            mFont,
            "#Exit",
            [this]{ mGuiManager.DoFade(.8, [this]{mGuiManager.ExitSimpleScreen(); }); }
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

        AddChildren();
    }

    void AddChildren()
    {
        AddChildBack(&mExitButton);
        AddChildBack(&mPlayerLocation);
        for (auto& t : mTowns)
            AddChildBack(&t);
    }

    unsigned ClassifyAngle(std::uint16_t bakAngle)
    {
        constexpr auto unit = 0xff / 8;
        return 4 * ((bakAngle / unit) % 8);
    }

    void UpdateLocation()
    {
        SetPlayerLocation(mGameState.GetZone(), mGameState.GetLocation());
    }

    void SetPlayerLocation(
        BAK::ZoneNumber zone,
        BAK::GamePositionAndHeading location)
    {
        const auto& [ss, ti, dims] = mIcons.GetFullMapIcon(ClassifyAngle(location.mHeading));
        mPlayerLocation.SetSpriteSheet(ss);
        mPlayerLocation.SetTexture(ti);
        mPlayerLocation.SetDimensions(dims);

        mPlayerLocation.SetCenter(
            mFMapXY.GetTileCoords(
                zone, 
                BAK::GetTile(location.mPosition)));
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;
    NullDialogScene mDialogScene;

    BAK::FMapXY mFMapXY;
    BAK::FMapTowns mFMapTowns;

    BAK::Layout mLayout;

    ClickButton mExitButton;

    Widget mPlayerLocation;
    std::vector<TownLabel> mTowns;

    const Logging::Logger& mLogger;
};

}
