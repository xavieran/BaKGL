#pragma once

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
        mDialogScene{},
        mFMapXY{},
        mFMapTowns{},
        mLayout{sLayoutFile},
        mExitButton{
            mLayout.GetWidgetLocation(sExitWidget),
            mLayout.GetWidgetDimensions(sExitWidget),
            mFont,
            "#Exit",
            [this]{ mGuiManager.ExitCharacterPortrait(); }
        },
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
        for (auto& t : mTowns)
            AddChildBack(&t);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    NullDialogScene mDialogScene;

    BAK::FMapXY mFMapXY;
    BAK::FMapTowns mFMapTowns;

    BAK::Layout mLayout;

    ClickButton mExitButton;

    std::vector<TownLabel> mTowns;

    const Logging::Logger& mLogger;
};

}
