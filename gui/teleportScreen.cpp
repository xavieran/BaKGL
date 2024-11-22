#include "gui/teleportScreen.hpp"

#include "audio/audio.hpp"

#include "bak/dialogSources.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/layout.hpp"
#include "bak/gameState.hpp"
#include "bak/money.hpp"
#include "bak/shop.hpp"
#include "bak/sounds.hpp"
#include "bak/temple.hpp"

#include "com/ostream.hpp"
#include "com/logger.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/icons.hpp"
#include "gui/fontManager.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

TeleportScreen::TeleportScreen(
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

    mTeleportFromText.SetText(mFont, "From:", true);
    mTeleportToText.SetText(mFont, "To:", true);
    mCostText.SetText(mFont, "Cost: ");
    mMapSnippet.AddChildBack(&mMap);

    AddChildren();
}

void TeleportScreen::DisplayNPCBackground() {}
void TeleportScreen::DisplayPlayerBackground() {}
void TeleportScreen::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
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
        auto factory = BAK::Encounter::TeleportFactory{};
        mGuiManager.DoTeleport(factory.Get(*mChosenDest - 1));
    }
}

void TeleportScreen::SetSourceTemple(unsigned sourceTemple, BAK::ShopStats* temple)
{
    mState = State::Idle;
    assert(temple);
    mTemple = temple;
    mSource = sourceTemple;
    mChosenDest = std::nullopt;
    for (unsigned i = 0; i < mTeleportDests.size(); i++)
    {
        if (sourceTemple - 1 == i)
        {
            mTeleportDests.at(i).SetSelected();
            mTeleportFromText.SetText(mFont, MakeTempleString("From:", sourceTemple), true);
        }
        else
        {
            mTeleportDests.at(i).SetUnselected();
        }

        mTeleportDests.at(i).SetCanReach(true);//mGameState.GetTempleSeen(i + 1));
    }

    AddChildren();
}

bool TeleportScreen::OnMouseEvent(const MouseEvent& me)
{
    const bool result = Widget::OnMouseEvent(me);

    if (mNeedRefresh)
    {
        AddChildren();
        mNeedRefresh = false;
    }

    return result;
}

void TeleportScreen::HandleTempleClicked(unsigned templeNumber)
{
    if (templeNumber == mSource)
    {
        mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportedToSameTemple, false, false, this);
    }
    else if (mGameState.GetChapter() == BAK::Chapter{6}
        && templeNumber == BAK::Temple::sChapelOfIshap
        && !mGameState.ReadEventBool(BAK::GameData::sPantathiansEventFlag))
    {
        mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportBlockedMalacsCrossDest, false, false, this);
    }
    else
    {
        const auto cost = CalculateCost(templeNumber);
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

void TeleportScreen::HandleTempleHighlighted(unsigned templeNumber, bool selected)
{
    if (selected)
    {
        mHighlightedDest = templeNumber;
        mTeleportToText.SetText(mFont, MakeTempleString("To:", templeNumber), true);
        mCostText.SetText(mFont, MakeCostString(templeNumber));
        mTeleportTo.SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(templeNumber - 1)));
    }
    else
    {
        mTeleportToText.SetText(mFont, "To:", true);
        mCostText.SetText(mFont, "Cost: ");
        mHighlightedDest.reset();
    }

    mNeedRefresh = true;
}

std::string TeleportScreen::MakeTempleString(const std::string& prefix, unsigned templeNumber)
{
    std::stringstream ss{};
    ss << prefix << "\n#" << mLayout.GetWidget(templeNumber - 1).mLabel;
    return ss.str();
}

std::string TeleportScreen::MakeCostString(unsigned templeNumber)
{
    std::stringstream ss{};
    ss << "Cost: " << BAK::ToShopDialogString(CalculateCost(templeNumber));
    return ss.str();
}

void TeleportScreen::AddChildren()
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

BAK::Royals TeleportScreen::CalculateCost(unsigned templeNumber)
{
    assert(mTemple);
    const auto srcPos = mLayout.GetWidget(mSource - 1).mPosition;
    const auto dstPos = mLayout.GetWidget(templeNumber - 1).mPosition;
    return BAK::Temple::CalculateTeleportCost(
        mSource,
        templeNumber,
        srcPos,
        dstPos,
        mTemple->mHaggleAnnoyanceFactor,
        mTemple->mCategories);
}

}
