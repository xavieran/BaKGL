#pragma once

#include "bak/coordinates.hpp"
#include "bak/camp.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/hotspot.hpp"
#include "bak/layout.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/compass.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"
#include "gui/scene.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class CampScreen : public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_CAMP.DAT";

    static constexpr auto sCampUntilHealed = 0;
    static constexpr auto sStop = 1;
    static constexpr auto sExit = 2;

    CampScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("ENCAMP.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
        mGuiManager{guiManager},
        mIcons{icons},
        mLayout{sLayoutFile},
        mButtons{},
        mLogger{Logging::LogState::GetLogger("Gui::Encamp")}
    {
        mButtons.reserve(mLayout.GetSize());
        for (unsigned i = 0; i < mLayout.GetSize(); i++)
        {
            mButtons.emplace_back(
                mLayout.GetWidgetLocation(i),
                mLayout.GetWidgetDimensions(i),
                std::get<Graphics::SpriteSheetIndex>(icons.GetButton(0)),
                std::get<Graphics::TextureIndex>(icons.GetButton(0)),
                std::get<Graphics::TextureIndex>(icons.GetPressedButton(0)),
                []{},
                []{});
        }

        auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer("ENCAMP.DAT");
        mDots.reserve(27*2);
        for (unsigned i = 0; i < 27; i++)
        {
            auto x = fb.GetUint16LE();
            auto y = fb.GetUint16LE();
            mDots.emplace_back(
                glm::vec2{x, y},
                glm::vec2{8, 3},
                std::get<Graphics::SpriteSheetIndex>(icons.GetEncampIcon(3)),
                std::get<Graphics::TextureIndex>(icons.GetEncampIcon(3)),
                std::get<Graphics::TextureIndex>(icons.GetEncampIcon(3)),
                []{},
                []{});

            mLogger.Debug() << x << " " << y << "\n";
        }
        for (unsigned i = 0; i < 27; i++)
        {
            continue;
            auto x = fb.GetUint16LE();
            auto y = fb.GetUint16LE();
            mDots.emplace_back(
                glm::vec2{x, y},
                glm::vec2{8, 3},
                std::get<Graphics::SpriteSheetIndex>(icons.GetEncampIcon(3)),
                std::get<Graphics::TextureIndex>(icons.GetEncampIcon(3)),
                std::get<Graphics::TextureIndex>(icons.GetEncampIcon(3)),
                []{},
                []{});

            mLogger.Debug() << x << " " << y << "\n";
        }

        AddChildren();

    }

    void AddChildren()
    {
        for (auto& button : mButtons)
            AddChildBack(&button);

        for (auto& dot : mDots)
            AddChildBack(&dot);
    }

private:
    IGuiManager& mGuiManager;
    const Icons& mIcons;

    BAK::Layout mLayout;

    BAK::CampData mCampData;
    std::vector<ClickButtonImage> mButtons;
    std::vector<ClickButtonImage> mDots;

    const Logging::Logger& mLogger;
};

}
