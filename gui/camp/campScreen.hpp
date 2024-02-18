#pragma once

#include "bak/camp.hpp"
#include "bak/dialogSources.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/layout.hpp"
#include "bak/shop.hpp"
#include "bak/time.hpp"

#include "graphics/texture.hpp"

#include "gui/core/clickable.hpp"
#include "gui/core/highlightable.hpp"

#include "gui/tickAnimator.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/camp/clock.hpp"
#include "gui/clickButton.hpp"
#include "gui/icons.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui::Camp {

class CampScreen : public Widget, public NullDialogScene
{
    enum class State
    {
        Idle,
        Camping,
        CampingTilHealed
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
        mCampData{},
        mFrame{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("DIALOG_BG_MAIN.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{320, 200},
            true
        },
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
        mPartyGold{
            {140, 90},
            {240, 60}
        },
        mButtons{},
        mDots{},
        mIsInInn{false},
        mState{State::Idle},
        mTimeElapser{nullptr},
        mLogger{Logging::LogState::GetLogger("Gui::Encamp")}
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

        mDots.reserve(mCampData.GetClockTicks().size());
        for (unsigned i = 0; i < mCampData.GetClockTicks().size(); i++)
        {
            auto dot = mDots.emplace_back(
                [this, i=i]{ HandleDotClicked(i); },
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

    void BeginCamp(bool isInn, BAK::ShopStats* shopStats)
    {
        const auto hour = mGameState.GetWorldTime().GetTime().GetHour();
        for (unsigned i = 0; i < mCampData.GetClockTicks().size(); i++)
        {
            mDots.at(i).SetCurrent(i == hour);
        }

        mIsInInn = isInn;
        assert(!isInn || shopStats);
        mShopStats = shopStats;

        SetText();
        AddChildren();

        if (mIsInInn)
        {
            ShowInnDialog(false);
        }
    }

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override
    {
        assert(choice);
        if (mGameState.GetEndOfDialogState() == -1 || choice->mValue == BAK::Keywords::sNoIndex)
        {
            Exit();
        }
        else
        {
            StartCamping(mShopStats->mInnSleepTilHour);
        }
    }


private:
    BAK::Royals GetInnCost()
    {
        assert(mShopStats);
        return BAK::GetRoyals(BAK::Sovereigns{mShopStats->mInnCost});
    }

    void ShowInnDialog(bool haveSlept)
    {
        auto sleepTil = mShopStats->mInnSleepTilHour;
        assert(sleepTil < mDots.size());
        mDots[sleepTil].SetHighlighted();

        mGameState.SetItemValue(GetInnCost());
        mGameState.SetDialogContext_7530(haveSlept);
        mGuiManager.StartDialog(
            BAK::DialogSources::mInnDialog, false, false, this);
    }

    void SetText()
    {
        std::stringstream namesSS{};
        namesSS << "\n";
        std::stringstream healthSS{};
        healthSS << "Health/Stamina\n";
        std::stringstream rationsSS{};
        rationsSS << "Rations\n";
        mGameState.GetParty().ForEachActiveCharacter(
            [&](auto& character){
                auto highlight = character.HaveNegativeCondition() ? "\xf5" : "";
                namesSS << highlight << character.GetName() << "\n";
                const auto health = character.GetSkill(BAK::SkillType::TotalHealth);
                const auto maxHealth = character.GetMaxSkill(BAK::SkillType::TotalHealth);
                // for the purposes of the highlight it's always 80%
                highlight = character.CanHeal(false) ? "\xf5" : " ";
                healthSS << highlight << " " << health << " " << highlight 
                    << " of " << maxHealth << "\n";
                const auto rations = character.GetTotalItem(
                    std::vector<BAK::ItemIndex>{BAK::sPoisonedRations, BAK::sRations, BAK::sSpoiledRations});
                const auto rhighlight = rations == 0 ? '\xf5' : ' ';
                rationsSS << rhighlight << " " << rations << " " << rhighlight << "\n";
                return BAK::Loop::Continue;
            });
        
        mNamesColumn.SetText(mFont, namesSS.str(), true, false, false, 1.5);
        mHealthColumn.SetText(mFont, healthSS.str(), true, false, false, 1.5);
        mRationsColumn.SetText(mFont, rationsSS.str(), true, false, false, 1.5);

        if (mIsInInn)
        {
            const auto partyGold = mGameState.GetParty().GetGold();
            const auto highlight = partyGold.mValue < GetInnCost().mValue ? '\xf5' : ' ';
            std::stringstream ss{};
            ss << "Party Gold:  " << highlight << BAK::ToShopString(partyGold);
            mPartyGold.SetText(mFont, ss.str());
        }
    }

    bool AnyCharacterCanHeal()
    {
        bool canHeal = false;
        mGameState.GetParty().ForEachActiveCharacter(
            [&](auto& character){
                canHeal |= character.CanHeal(mIsInInn);
                canHeal |= character.HaveNegativeCondition();
                return BAK::Loop::Continue;
            });
        return canHeal;
    }

    void HandleDotClicked(unsigned i)
    {
        StartCamping(i);
    }

    unsigned GetHour()
    {
        return mGameState.GetWorldTime().GetTime().GetHour();
    }

    void StartCamping(std::optional<unsigned> hourTil)
    {
        mTargetHour = hourTil;
        mTimeBeganCamping = mGameState.GetWorldTime().GetTime();

        if ((!mIsInInn && hourTil && mDots.at(*hourTil).GetCurrent())
            || mState == State::Camping)
        {
            return;
        }

        const auto it = std::find_if(mDots.begin(), mDots.end(),
            [](const auto& dot){ return dot.GetCurrent(); });
        assert(it != mDots.end());

        mState = hourTil ? State::Camping : State::CampingTilHealed;

        auto timeElapser = std::make_unique<TickAnimator>(
            .02,
            [this]{
                this->HandleTick();
            });
        mTimeElapser = timeElapser.get();
        mGuiManager.AddAnimator(std::move(timeElapser));
        AddChildren();
    }

    void HandleTick()
    {
        auto camp = BAK::TimeChanger(mGameState);
        camp.ElapseTimeInSleepView(
            BAK::Times::OneHour,
            mIsInInn ? 0x85 : 0x64,
            mIsInInn ? 0x64 : 0x50);

        if ((mGameState.GetWorldTime().GetTime() - mTimeBeganCamping) > BAK::Times::ThirteenHours)
        {
            mGameState.GetParty().ForEachActiveCharacter([&](auto& character){
                character.AdjustCondition(BAK::Condition::Sick, -100);
                return BAK::Loop::Continue;
            });
        }
        bool isLast = GetHour() == mTargetHour;

        if (isLast || (mState == State::CampingTilHealed && !AnyCharacterCanHeal()))
        {
            FinishedTicking();
        }

        if (mState == State::CampingTilHealed || mIsInInn)
        {
            for (unsigned i = 0; i < mDots.size(); i++)
            {
                mDots.at(i).SetCurrent(false);
            }
        }

        mDots.at(GetHour()).SetCurrent(true);
        SetText();
    }

    void FinishedTicking()
    {
        if (mTimeElapser)
        {
            mTimeElapser->Stop();
        }

        mTimeElapser = nullptr;

        for (unsigned i = 0; i < mCampData.GetClockTicks().size(); i++)
        {
            mDots.at(i).SetCurrent(i == GetHour());
        }

        const auto prevState = mState;
        mState = State::Idle;
        AddChildren();

        if (mIsInInn)
        {
            mGameState.GetParty().LoseMoney(BAK::GetRoyals(BAK::Sovereigns{mShopStats->mInnCost}));
            if (!AnyCharacterCanHeal())
            {
                Exit();
            }
            else
            {
                ShowInnDialog(true);
            }
        }
        else if (prevState != State::CampingTilHealed)
        {
            Exit();
        }

    }

    void HandleButton(unsigned button)
    {
        if (button == sCampUntilHealed)
        {
            const auto hour = mGameState.GetWorldTime().GetTime().GetHour();
            mLogger.Spam() << "Hour: "<< hour << "\n";
            StartCamping(std::nullopt);
        }
        else if (button == sStop)
        {
            if (mTimeElapser != nullptr)
            {
                FinishedTicking();
            }
        }
        else if (button == sExit)
        {
            Exit();
        }
    }

    void Exit()
    {
        // exit lock happens to do exactly what I want.. should probably rename it
        mGuiManager.DoFade(.8, [this]{mGuiManager.ExitLock(); });
    }

    void AddChildren()
    {
        ClearChildren();

        if (!mIsInInn)
        {
            if (mState == State::Camping || mState == State::CampingTilHealed)
            {
                AddChildBack(&mButtons[sStop]);
            }
            else
            {
                if (AnyCharacterCanHeal())
                {
                    AddChildBack(&mButtons[sCampUntilHealed]);
                }

                AddChildBack(&mButtons[sExit]);
            }
        }

        for (auto& dot : mDots)
            AddChildBack(&dot);

        AddChildBack(&mNamesColumn);
        AddChildBack(&mHealthColumn);
        AddChildBack(&mRationsColumn);

        if (mIsInInn)
        {
            AddChildBack(&mPartyGold);
            AddChildBack(&mFrame);
            SetInactive();
        }
        else
        {
            SetActive();
        }

    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;
    BAK::CampData mCampData;

    Widget mFrame;
    TextBox mNamesColumn;
    TextBox mHealthColumn;
    TextBox mRationsColumn;
    TextBox mPartyGold;

    std::vector<ClickButtonImage> mButtons;
    using ClockTick = Highlightable<
        Clickable<
            detail::CampDest,
            LeftMousePress,
            std::function<void()>>,
        true>;
    std::vector<ClockTick> mDots;

    bool mIsInInn;
    State mState;
    std::optional<unsigned> mTargetHour;
    BAK::Time mTimeBeganCamping;
    TickAnimator* mTimeElapser;
    BAK::ShopStats* mShopStats;

    const Logging::Logger& mLogger;
};

}
