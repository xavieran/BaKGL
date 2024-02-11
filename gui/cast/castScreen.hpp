#pragma once

#include "bak/fileBufferFactory.hpp"
#include "bak/layout.hpp"

#include "graphics/texture.hpp"

#include "gui/cast/symbol.hpp"

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui::Cast {

class CastScreen : public Widget, public NullDialogScene
{
    enum class State
    {
        Idle,
        SpellSelected,
    };
    static constexpr auto sLayoutFile = "REQ_CAST.DAT";
    static constexpr auto sCombatLayoutFile = "SPELL.DAT";
    static constexpr auto sScreen = "CAST.SCX";

    static constexpr auto sSymbol5 = 0;
    static constexpr auto sSymbol6 = 1;
    static constexpr auto sExit = 6;

public:
    CastScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        const Font& spellFont,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sScreen),
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
        mSymbol{spellFont},
        mButtons{},
        mState{State::Idle},
        mLogger{Logging::LogState::GetLogger("Gui::Cast")}
    {
        mButtons.reserve(mLayout.GetSize());
        for (unsigned i = 0; i < mLayout.GetSize(); i++)
        {
            const auto& w = mLayout.GetWidget(i);
            mButtons.emplace_back(
                mLayout.GetWidgetLocation(i),
                mLayout.GetWidgetDimensions(i),
                std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(w.mImage)),
                std::get<Graphics::TextureIndex>(mIcons.GetButton(w.mImage)),
                std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(w.mImage)),
                [this, i]{ HandleButton(i); },
                []{});
        }

        /*
        for (unsigned i = 0; i < mCastData.GetClockTicks().size(); i++)
        {
            auto dot = mDots.emplace_back(
                [this, i=i]{ HandleDotClicked(i); },
                mIcons,
                mCastData.GetClockTicks().at(i),
                [](bool selected){});
            dot.SetCurrent(false);
        }
        */

        AddChildren();
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        bool handled = false;
        //if (mState == State::Idle)
        //{
        //    for (auto& widget : mDots)
        //    {
        //        handled |= widget.OnMouseEvent(event);
        //    }
        //}

        for (auto& widget : mButtons)
        {
            handled |= widget.OnMouseEvent(event);
        }

        return handled;
    }

    void BeginCast(bool inCombat)
    {
        AddChildren();
    }

private:
    void Exit()
    {
        // exit lock happens to do exactly what I want.. should probably rename it
        mGuiManager.DoFade(.8, [this]{mGuiManager.ExitLock(); });
    }

    void HandleButton(unsigned i)
    {
        if (i == sExit)
        {
            Exit();
        }
        else if (i == sSymbol5)
        {
            mSymbol.SetSymbol(5);
        }
        else if (i == sSymbol6)
        {
            mSymbol.SetSymbol(6);
        }
        AddChildren();
    }

    void AddChildren()
    {
        ClearChildren();
        for (auto& widget : mButtons)
        {
            AddChildBack(&widget);
        }

        AddChildBack(&mSymbol);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;

    Symbol mSymbol;
    std::vector<ClickButtonImage> mButtons;
    State mState;
    std::optional<unsigned> mTargetHour;
    BAK::Time mTimeBeganCasting;
    BAK::ShopStats* mShopStats;

    const Logging::Logger& mLogger;
};

}
