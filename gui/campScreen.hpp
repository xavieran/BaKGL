#pragma once

#include "bak/camp.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/layout.hpp"

#include "graphics/texture.hpp"

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"

#include "gui/IAnimator.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

namespace detail {

class TimeElapser : public IAnimator
{
    static constexpr auto sTickSpeed = .1;
public:
    TimeElapser(
        unsigned hourBegin,
        unsigned hourEnd,
        std::function<void(unsigned, bool)>&& callback)
    :
        mAccumulatedTimeDelta{},
        mAlive{true},
        mHour{hourBegin},
        mHourEnd{hourEnd},
        mCallback{std::move(callback)}
    {
        assert(mHour != mHourEnd);
    }

    void OnTimeDelta(double delta) override
    {
        mAccumulatedTimeDelta += delta;
        Logging::LogDebug(__FUNCTION__) << " " << delta << " " << mAccumulatedTimeDelta
            << " " << mHour << " " << mAlive << "\n";

        if (mAccumulatedTimeDelta > sTickSpeed && mAlive)
        {
            mAccumulatedTimeDelta = 0;
            mHour += 1;
            mHour %= 24;
            mCallback(mHour, mHour == mHourEnd);
        }
    }

    bool IsAlive() const override
    {
        return mAlive && mHour != mHourEnd;
    }

    void Stop()
    {
        mAlive = false;
    }

private:
    double mAccumulatedTimeDelta;
    bool mAlive;
    unsigned mHour;
    const unsigned mHourEnd;
    std::function<void(unsigned, bool)> mCallback;
};

class CampDest : public Widget
{
    static constexpr auto sBlank = 0;
    static constexpr auto sUnlit = 1;
    static constexpr auto sHighlighted = 2;
    static constexpr auto sCurrent = 3;
    static constexpr auto sBlank2 = 4;

public:
    CampDest(
        const Icons& icons,
        glm::vec2 pos,
        std::function<void(bool)>&& selected)
    :
        Widget{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetEncampIcon(sUnlit)),
            std::get<Graphics::TextureIndex>(icons.GetEncampIcon(sUnlit)),
            pos,
            {8, 3},
            false
        },
        mIcons{icons},
        mCurrent{},
        mCallback{selected}
    {
    }

    bool GetCurrent() const
    {
        return mCurrent;
    }

    void SetCurrent(bool current)
    {
        mCurrent = current;
        SetTexture(std::get<Graphics::TextureIndex>(
            mCurrent
                ? mIcons.GetEncampIcon(sCurrent)
                : mIcons.GetEncampIcon(sUnlit)));
    }

public:
    void Entered()
    {
        SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetEncampIcon(sHighlighted)));
        mCallback(true);
    }

    void Exited()
    {
        SetCurrent(mCurrent);
        mCallback(false);
    }

    const Icons& mIcons;
    bool mCurrent;
    std::function<void(bool)> mCallback;
};

}
class CampScreen : public Widget
{
    enum class State
    {
        Idle,
        Camping
    };
    static constexpr auto sLayoutFile = "REQ_CAMP.DAT";
    static constexpr auto sScreen = "ENCAMP.SCX";

    static constexpr auto sCampUntilHealed = 0;
    static constexpr auto sStop = 1;
    static constexpr auto sExit = 2;

public:
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
        mNamesColumn{
            {130, 20},
            {60, 200}
        },
        mHealthColumn{
            {180, 20},
            {70, 200}
        },
        mRationsColumn{
            {250, 20},
            {60, 200}
        },
        mButtons{},
        mDots{},
        mIsInInn{false},
        mState{State::Idle},
        mLatestTick{},
        mTimeElapser{nullptr},
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

        mDots.reserve(mCampData.GetClockTicks().size());
        for (unsigned i = 0; i < mCampData.GetClockTicks().size(); i++)
        {
            auto dot = mDots.emplace_back(
                [this, i=i]{ HandleDot(i); },
                mIcons,
                mCampData.GetClockTicks().at(i),
                [](bool selected){});
            dot.SetCurrent(false);
        }

        AddChildren();
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        bool handled = false;
        if (mState == State::Idle)
        {
            for (auto& widget : mDots)
            {
                handled |= widget.OnMouseEvent(event);
            }
        }

        for (auto& widget : mButtons)
        {
            handled |= widget.OnMouseEvent(event);
        }

        return handled;
    }

    void SetIsInn(bool isInn)
    {
        const auto hour = mGameState.GetWorldTime().mTime.GetHour();
        for (unsigned i = 0; i < mCampData.GetClockTicks().size(); i++)
        {
            mDots.at(i).SetCurrent(i == hour);
        }
        mIsInInn = isInn;
        AddText();
        AddChildren();
    }

private:
    void AddText()
    {
        std::stringstream namesSS{};
        namesSS << "\n";
        std::stringstream healthSS{};
        healthSS << "Health/Stamina\n";
        std::stringstream rationsSS{};
        rationsSS << "Rations\n";
        mGameState.GetParty().ForEachActiveCharacter(
            [&](auto& character){
                namesSS << character.GetName() << "\n";
                const auto health = character.GetSkill(BAK::SkillType::TotalHealth);
                const auto maxHealth = character.GetMaxSkill(BAK::SkillType::TotalHealth);
                const auto multiplier = mIsInInn ? 1.0 : .70;
                const auto highlight = health < (maxHealth * multiplier) ? '\xf5' : ' ';
                healthSS << highlight << " " << health << " " << highlight 
                    << " of " << maxHealth << "\n";
                const auto rations = character.GetTotalItem(
                    std::vector<BAK::ItemIndex>{BAK::sPoisonedRations, BAK::sRations, BAK::sSpoiledRations});
                const auto rhighlight = rations == 0 ? '\xf5' : ' ';
                rationsSS << rhighlight << " " << rations << " " << rhighlight << "\n";
                return false;
            });
        mLogger.Debug() << namesSS.str() << "fin\n";
        mLogger.Debug() << healthSS.str() << "fin\n";
        mLogger.Debug() << rationsSS.str() << "fin\n";
        mNamesColumn.AddText(mFont, namesSS.str(), true, false, false, 1.5);
        mHealthColumn.AddText(mFont, healthSS.str(), true, false, false, 1.5);
        mRationsColumn.AddText(mFont, rationsSS.str(), true, false, false, 1.5);
    }

    void HandleDot(unsigned i)
    {
        mLogger.Debug() << "Hour: " << i << "\n";
        if (mDots.at(i).GetCurrent() || mState == State::Camping)
        {
            return;
        }

        const auto it = std::find_if(mDots.begin(), mDots.end(),
            [](const auto& dot){ return dot.GetCurrent(); });
        assert(it != mDots.end());

        mState = State::Camping;

        auto timeElapser = std::make_unique<detail::TimeElapser>(
            std::distance(mDots.begin(), it),
            i,
            [this](unsigned index, bool isLast){
                mLogger.Debug() << "HourTicked: " << index << "\n";
                if (isLast)
                {
                    FinishedTicking(index);
                }
                mDots.at(index).SetCurrent(true);
                mLatestTick = index;
            });
        mTimeElapser = timeElapser.get();
        mGuiManager.AddAnimator(std::move(timeElapser));
        AddChildren();
    }

    void FinishedTicking(unsigned endTick)
    {
        mTimeElapser = nullptr;
        for (unsigned i = 0; i < mDots.size(); i++)
        {
            mDots.at(i).SetCurrent(false);
        }
        mDots.at(endTick).SetCurrent(true);
        mState = State::Idle;
        AddChildren();
    }

    void HandleButton(unsigned button)
    {
        if (button == sCampUntilHealed)
        {
            const auto hour = mGameState.GetWorldTime().mTime.GetHour();
            mLogger.Debug() << "Hour: "<< hour << "\n";
        }
        else if (button == sStop)
        {
            if (mTimeElapser != nullptr)
            {
                mTimeElapser->Stop();
                FinishedTicking(mLatestTick);
            }
        }
        else if (button == sExit)
        {
            mGuiManager.DoFade(.8, [this]{mGuiManager.ExitSimpleScreen(); });
        }
    }

    std::string GetButtonText(unsigned button) const
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
        ClearChildren();

        if (mState == State::Camping)
        {
            AddChildBack(&mButtons[sStop]);
        }
        else
        {
            AddChildBack(&mButtons[sCampUntilHealed]);
            AddChildBack(&mButtons[sExit]);
        }

        for (auto& dot : mDots)
            AddChildBack(&dot);

        AddChildBack(&mNamesColumn);
        AddChildBack(&mHealthColumn);
        AddChildBack(&mRationsColumn);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;
    BAK::CampData mCampData;

    TextBox mNamesColumn;
    TextBox mHealthColumn;
    TextBox mRationsColumn;
    std::vector<ClickButton> mButtons;
    using ClockTick = Highlightable<Clickable<detail::CampDest, LeftMousePress, std::function<void()>>, true>;
    std::vector<ClockTick> mDots;

    bool mIsInInn;
    State mState;
    unsigned mLatestTick;
    detail::TimeElapser* mTimeElapser;

    const Logging::Logger& mLogger;
};

}
