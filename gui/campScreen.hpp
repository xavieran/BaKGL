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
#include "gui/core/clickable.hpp"
#include "gui/highlightable.hpp"
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
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState)
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
        mFont{font},
        mGameState{gameState},
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
                mFont,
                GetButtonText(i),
                [this, i]{ HandleButton(i); });
        }

        mDots.reserve(24);
        for (unsigned i = 0; i < 24; i++)
        {
            mDots.emplace_back(
                []{},
                ImageTag{},
                std::get<Graphics::SpriteSheetIndex>(icons.GetEncampIcon(2)),
                std::get<Graphics::TextureIndex>(icons.GetEncampIcon(3)),
                mCampData.GetClockTicks().at(i),
                glm::vec2{8, 3},
                true);
        }

        AddChildren();
    }

    void SetIsInn(bool isInn)
    {
    }

private:

    void HandleButton(unsigned button)
    {
        if (button == sCampUntilHealed)
        {
        }
        else if (button == sStop)
        {
        }
        else if (button == sExit)
        {
            mGuiManager.DoFade(.8, [this]{mGuiManager.ExitSimpleScreen(); });
        }
    }

    std::string GetButtonText(unsigned button)
    {
        if (button == sCampUntilHealed)
        {
            return "Camp until Healed";
        }
        else if (button == sStop)
        {
            return "Stop";
        }
        else if (button == sExit)
        {
            return "Exit";
        }
        assert(false);
        return "";
    }

    void AddChildren()
    {
        for (auto& button : mButtons)
            AddChildBack(&button);

        for (auto& dot : mDots)
            AddChildBack(&dot);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;

    BAK::CampData mCampData;
    std::vector<ClickButton> mButtons;
    class ClockTickBase : public Widget
    {
    public:
        using Widget::Widget;
        void Entered(){}
        void Exited(){}
    };
    using ClockTick = Highlightable<Clickable<ClockTickBase, LeftMousePress, std::function<void()>>, true>;
    std::vector<ClockTick> mDots;

    const Logging::Logger& mLogger;
};

}
