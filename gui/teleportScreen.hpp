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
#include "gui/teleportDest.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class TeleportScreen : public Widget
{
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
        mDialogScene{},
        mLayout{sLayoutFile},
        mSource{},
        mHighlightedDest{},
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
            // FIXME: This callback works by accident, rename it
            [this]{ mGuiManager.ExitCharacterPortrait(); }
        },
        mTeleportDests{},
        mLogger{Logging::LogState::GetLogger("Gui::TeleportScreen")}
    {
        mTeleportDests.reserve(mLayout.GetSize());

        for (unsigned i = 0; i < mLayout.GetSize() - 1; i++)
        {
            mTeleportDests.emplace_back(
                icons,
                mLayout.GetWidgetLocation(i),
                [this, i=i](bool selected){
                    HandleTempleHighlighted(i + 1, selected);
                },
                [this, i=i]{
                    HandleTempleClicked(i + 1);
                }
            );
        }

        mTeleportFromText.AddText(mFont, "From:", true);
        mTeleportToText.AddText(mFont, "To:\n#Temple of Ishap", true);
        mCostText.AddText(mFont, "Cost: #112 sovereigns");
        mMapSnippet.AddChildBack(&mMap);

        AddChildren();
    }

    void SetSourceTemple(unsigned sourceTemple)
    {
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
        }
    }

    void HandleTempleClicked(unsigned templeNumber)
    {
        mLogger.Debug() << "Clicked temple: " << templeNumber << "\n";
    }

    void HandleTempleHighlighted(unsigned templeNumber, bool selected)
    {
        mLogger.Debug() << "Entered temple: " << templeNumber << " " << selected << "\n";
        if (selected)
        {
            mHighlightedDest = templeNumber;
            mTeleportToText.AddText(mFont, MakeTempleString("To:", templeNumber), true);
            mTeleportTo.SetTexture(std::get<Graphics::TextureIndex>(mIcons.GetTeleportIcon(templeNumber - 1)));
        }
        else
        {
            mTeleportToText.AddText(mFont, "To:", true);
            mHighlightedDest.reset();
        }

        AddChildren();
    }

    std::string MakeTempleString(const std::string& prefix, unsigned templeNumber)
    {
        std::stringstream ss{};
        ss << prefix << "\n#" << mLayout.GetWidget(templeNumber - 1).mLabel;
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
            AddChildBack(&dst);
        }

        AddChildBack(&mCancelButton);
    }

private:
    IGuiManager& mGuiManager;
    const Font& mFont;
    BAK::GameState& mGameState;
    const Icons& mIcons;
    NullDialogScene mDialogScene;

    BAK::Layout mLayout;

    unsigned mSource;
    std::optional<unsigned> mHighlightedDest;

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
