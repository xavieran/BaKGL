#pragma once

#include "audio/audio.hpp"

#include "bak/dialogSources.hpp"
#include "bak/layout.hpp"
#include "bak/sounds.hpp"
#include "bak/temple.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/teleportDest.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class TeleportScreen : public Widget, public IDialogScene
{
    enum class State
    {
        Idle,
        Teleported,
        Cancelled
    };

public:
    static constexpr auto sLayoutFile = "REQ_TELE.DAT";
    static constexpr auto sCancelWidget = 12;

    TeleportScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        BAK::GameState& gameState)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("DIALOG.SCX"),
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
        mState{State::Idle},
        mSource{},
        mHighlightedDest{},
        mChosenDest{},
        mNeedRefresh{false},
        mTeleportWord{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetTeleportIcon(12)),
            std::get<Graphics::TextureIndex>(icons.GetTeleportIcon(12)),
            glm::vec2{30, 10},
            std::get<glm::vec2>(icons.GetTeleportIcon(12)),
            false
        },
        mTeleportFromText{
            glm::vec2{30, 36},
           glm::vec2{80, 40}
        },
        mTeleportFrom{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetTeleportIcon(0)),
            std::get<Graphics::TextureIndex>(icons.GetTeleportIcon(0)),
            glm::vec2{40, 58},
            std::get<glm::vec2>(icons.GetTeleportIcon(0)),
            false
        },
        mTeleportToText{
            glm::vec2{30, 108},
            glm::vec2{80, 40}
        },
        mTeleportTo{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(icons.GetTeleportIcon(1)),
            std::get<Graphics::TextureIndex>(icons.GetTeleportIcon(1)),
            glm::vec2{40, 130},
            std::get<glm::vec2>(icons.GetTeleportIcon(1)),
            false
        },
        mCostText{
            glm::vec2{18, 180},
            glm::vec2{180, 40}
        },
        mMapSnippet{
            ClipRegionTag{},
            glm::vec2{127, 15},
            glm::vec2{172, 160},
            false 
        },
        mMap{
            Graphics::DrawMode::Sprite,
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen("FULLMAP.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{23, 2},
            glm::vec2{320, 200},
            true
        },
        mCancelButton{
            mLayout.GetWidgetLocation(sCancelWidget),
            mLayout.GetWidgetDimensions(sCancelWidget),
            mFont,
            "#Cancel",
            [this]{ 
                if (mState != State::Cancelled)
                {
                    mState = State::Cancelled;
                    mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogCancel, false, false, this);
                }
            }
        },
        mTeleportDests{},
        mLogger{Logging::LogState::GetLogger("Gui::TeleportScreen")}
    {
        mTeleportDests.reserve(mLayout.GetSize());

        for (unsigned i = 0; i < mLayout.GetSize() - 1; i++)
        {
            mTeleportDests.emplace_back(
                [this, i=i]{
                    HandleTempleClicked(i + 1);
                },
                icons,
                mLayout.GetWidgetLocation(i),
                [this, i=i](bool selected){
                    HandleTempleHighlighted(i + 1, selected);
                }
            );
        }

        mTeleportFromText.AddText(mFont, "From:", true);
        mTeleportToText.AddText(mFont, "To:", true);
        mCostText.AddText(mFont, "Cost: ");
        mMapSnippet.AddChildBack(&mMap);

        AddChildren();
    }

    void DisplayNPCBackground() override {}
    void DisplayPlayerBackground() override {}

    void DialogFinished(const std::optional<BAK::ChoiceIndex>& choice) override
    {
        mLogger.Debug() << "Finished dialog with choice : " << choice << "\n";
        if (mState == State::Cancelled)
        {
            mGuiManager.DoFade(.8, [this]{
                mGuiManager.ExitSimpleScreen();
            });
        }
        else if (mState == State::Teleported)
        {
            ASSERT(mChosenDest);
            mGuiManager.ExitSimpleScreen();
            // This is probably a hack, likely need to fix this for other teleport things..
            AudioA::AudioManager::Get().PopTrack();
            mGuiManager.DoTeleport(BAK::TeleportIndex{*mChosenDest - 1});
        }
    }

    void SetSourceTemple(unsigned sourceTemple)
    {
        mState = State::Idle;
        mSource = sourceTemple;
        mChosenDest = std::nullopt;
        for (unsigned i = 0; i < mTeleportDests.size(); i++)
        {
            if (sourceTemple - 1 == i)
            {
                mTeleportDests.at(i).SetSelected();
                mTeleportFromText.AddText(mFont, MakeTempleString("From:", sourceTemple), true);
            }
            else
            {
                mTeleportDests.at(i).SetUnselected();
            }

            mTeleportDests.at(i).SetCanReach(true);//mGameState.GetTempleSeen(i + 1));
        }

        AddChildren();
    }

	bool OnMouseEvent(const MouseEvent& me) override
	{
		const bool result = Widget::OnMouseEvent(me);

        if (mNeedRefresh)
        {
            AddChildren();
            mNeedRefresh = false;
        }

        return result;
	}

private:
    void HandleTempleClicked(unsigned templeNumber)
    {
        if (templeNumber == mSource)
        {
            mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportedToSameTemple, false, false, this);
        }
        else if (mGameState.GetChapter() == BAK::Chapter{6}
            && templeNumber == BAK::Temple::sChapelOfIshap
            && !mGameState.GetEventStateBool(BAK::GameData::sPantathiansEventFlag))
        {
            mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportBlockedMalacsCrossDest, false, false, this);
        }
        else
        {
            const auto cost = BAK::Temple::CalculateTeleportCost(mSource, templeNumber);
            if (cost < mGameState.GetMoney())
            {
                mGameState.GetParty().LoseMoney(cost);
                mState = State::Teleported;
                mChosenDest = templeNumber;
                AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{BAK::sTeleportSound});
                mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogPostTeleport, false, false, this);
            }
            else
            {
                mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogCantAfford, false, false, this);
            }
        }
    }

    void HandleTempleHighlighted(unsigned templeNumber, bool selected)
    {
        if (selected)
        {
            mHighlightedDest = templeNumber;
            mTeleportToText.AddText(mFont, MakeTempleString("To:", templeNumber), true);
            mCostText.AddText(mFont, MakeCostString(templeNumber));
            mTeleportTo.SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(templeNumber - 1)));
        }
        else
        {
            mTeleportToText.AddText(mFont, "To:", true);
            mCostText.AddText(mFont, "Cost: ");
            mHighlightedDest.reset();
        }

        mNeedRefresh = true;
    }

    std::string MakeTempleString(const std::string& prefix, unsigned templeNumber)
    {
        std::stringstream ss{};
        ss << prefix << "\n#" << mLayout.GetWidget(templeNumber - 1).mLabel;
        return ss.str();
    }

    std::string MakeCostString(unsigned templeNumber)
    {
        std::stringstream ss{};
        ss << "Cost: " << BAK::ToShopDialogString(BAK::Temple::CalculateTeleportCost(mSource, templeNumber));
        return ss.str();
    }

    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mMapSnippet);

        AddChildBack(&mTeleportWord);

        AddChildBack(&mTeleportFromText);
        AddChildBack(&mTeleportFrom);

        AddChildBack(&mTeleportToText);
        if (mHighlightedDest)
        {
            AddChildBack(&mTeleportTo);
        }

        AddChildBack(&mCostText);

        for (auto& dst : mTeleportDests)
        {
			if (dst.IsCanReach())
                AddChildBack(&dst);
        }

        AddChildBack(&mCancelButton);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;

    BAK::Layout mLayout;

    State mState;
    unsigned mSource;
    std::optional<unsigned> mHighlightedDest;
    std::optional<unsigned> mChosenDest;
    bool mNeedRefresh;

    Widget mTeleportWord;
    TextBox mTeleportFromText;
    Widget mTeleportFrom;
    TextBox mTeleportToText;
    Widget mTeleportTo;
    TextBox mCostText;
    Widget mMapSnippet;
    Widget mMap;
    ClickButton mCancelButton;

    std::vector<TeleportDest> mTeleportDests;

    const Logging::Logger& mLogger;
};

}
