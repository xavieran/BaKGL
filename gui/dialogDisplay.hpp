#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/algorithm.hpp"
#include "com/assert.hpp"
#include "com/visit.hpp"

#include "gui/IDialogDisplay.hpp"
#include "gui/IDialogScene.hpp"
#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/button.hpp"
#include "gui/colors.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <string>
#include <string_view>

namespace Gui {

enum class DialogFrame
{
    Fullscreen = 0,
    ActionArea = 1,
    ActionAreaInventory = 2,
    LowerArea = 3,
    Popup = 4
};

class DialogDisplay : public Widget, IDialogDisplay
{
public:
    DialogDisplay(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Backgrounds& bgs,
        const Font& fr,
        BAK::GameState& gameState)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            glm::vec4{0},
            true
        },
        mKeywords{},
        mGameState{gameState},
        mCenter{160, 112},
        mFont{fr},
        mActors{actors},
        mLabel{
            mCenter,
            glm::vec2{320, 20},
            mFont,
            "#LABEL#"},
        mActor{
            Graphics::DrawMode::Sprite,
            mActors.GetSpriteSheet(),
            Graphics::TextureIndex{},
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{100, 19},
            glm::vec2{0},
            true},
        mFullscreenFrame{
            Graphics::DrawMode::Sprite,
            bgs.GetSpriteSheet(),
            bgs.GetScreen("OPTIONS2.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            true
        },
        mWorldViewFrame{
            Graphics::DrawMode::Sprite,
            bgs.GetSpriteSheet(),
            bgs.GetScreen("DIALOG_BG_MAIN.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            true
        },
        mActionAreaFrame{
            ClipRegionTag{},
            glm::vec2{15, 11},
            glm::vec2{289, 101},
            true
        },
        mActionAreaBackground{
            Graphics::DrawMode::Sprite,
            bgs.GetSpriteSheet(),
            bgs.GetScreen("DIALOG.SCX"),
            Graphics::ColorMode::Texture,
            glm::vec4{0},
            glm::vec2{-15, -11},
            glm::vec2{320, 200},
            true
        },
        mLowerFrame{
            RectTag{},
            glm::vec2{15, 125},
            glm::vec2{285, 66},
            glm::vec4{0},
            true
        },
        mFullscreenTextBox{
            glm::vec2{30, 30},
            glm::vec2{320 - 30*2, 135}
        },
        mActionAreaTextBox{
            glm::vec2{6, 6},
            glm::vec2{290, 103}
        },
        mLowerTextBox{
            glm::vec2{0, 0},
            glm::vec2{285, 66}
        },
        mPopup{
            glm::vec2{},
            glm::vec2{},
            Color::buttonBackground,
            Color::buttonHighlight,
            Color::buttonShadow,
            Color::black
        },
        mPopupText{
            glm::vec2{},
            glm::vec2{}
        },
        mLogger{Logging::LogState::GetLogger("Gui::DialogDisplay")}
    {
        mFullscreenFrame.AddChildBack(&mFullscreenTextBox);

        mLowerFrame.AddChildBack(&mLowerTextBox);

        mActionAreaFrame.AddChildBack(&mActionAreaBackground);
        mActionAreaFrame.AddChildBack(&mActionAreaTextBox);

        mPopup.AddChildBack(&mPopupText);
    }

    void ShowWorldViewPane(bool isInWorldView)
    {
        if (isInWorldView)
            AddChildBack(&mWorldViewFrame);
    }

    void DisplayPlayer(IDialogScene& dialogScene, unsigned act)
    {
        const auto actor = mGameState.GetActor(act);
        if (actor)
        {
            SetActor(*actor, dialogScene, false);
            AddLabel("#" 
                + std::string{mKeywords.GetNPCName(*actor)}
                + " asked about:#");
        }
        else
        {
        }
    }

    std::pair<glm::vec2, std::string> DisplaySnippet(
        IDialogScene& dialogScene,
        const BAK::DialogSnippet& snippet,
        std::string_view remainingText,
        bool isInWorldView,
        glm::vec2 mousePos)
    {
        ClearChildren();

        auto popup = snippet.GetPopup();
        if (popup)
        {
            // Need a more reliable way of getting mouse pos
            //mPopup.SetPosition(mousePos);
            mPopup.SetPosition(popup->mDims);
            mPopup.SetDimensions(popup->mDims);
            mPopupText.SetPosition(glm::vec2{1});
            mPopupText.SetDimensions(popup->mDims);
        }

        std::string text{remainingText};

        text = mGameState.GetTextVariableStore().SubstituteVariables(text);

        const auto ds1 = snippet.mDisplayStyle;
        const auto ds2 = snippet.mDisplayStyle2;
        const auto ds3 = snippet.mDisplayStyle3;
        const auto act = snippet.mActor;

        const auto dialogFrame = std::invoke([&]{
            if (popup)
                return DialogFrame::Popup;
            else if (act != 0x0)
                return DialogFrame::LowerArea;
            else if (ds1 == 0x06)
                return DialogFrame::Fullscreen;
            else if (ds1 == 0x05)
                return DialogFrame::ActionAreaInventory;
            else if (ds1 == 0x03 || ds1 == 0x04)
                return DialogFrame::LowerArea;
            else if (ds1 == 0x02 || ds2 == 0x14 || ds3 == 0x02 || ds2 == 0x10)
                return DialogFrame::ActionArea;
            else
                return DialogFrame::Fullscreen;
        });

        const bool verticallyCentered
            = ((ds2 & 0x10) != 0) || (ds2 == 0x3);
        const bool horizontallyCentered 
            = (ds2 & 0x4) == 0x4;
        const bool isBold
            = ds2 == 0x3;

        std::optional<unsigned> actor{};

        if (act != 0)
        {
            actor = mGameState.GetActor(act);
        }

        const auto [charPos, undisplayedText] = SetText(
            text,
            dialogFrame,
            horizontallyCentered,
            verticallyCentered,
            isBold,
            isInWorldView);

        if (actor)
        {
            SetActor(*actor, dialogScene, true);
        }

        return std::make_pair(charPos, std::string{undisplayedText});
    }

    void ShowFlavourText(BAK::Target target)
    {
        const auto& snippet = BAK::DialogStore::Get().GetSnippet(target);
        const auto text = snippet.GetText();
        const auto flavourText = find_nth(text.begin(), text.end(), '#', 2);
        ASSERT(flavourText != text.end());
        const auto remainingText = std::string{flavourText, text.end()};
        auto nullScene = NullDialogScene{};
        DisplaySnippet(nullScene, snippet, remainingText, false, glm::vec2{});

        const auto label = std::string{text.begin(), flavourText};
        AddLabel(label);
    }

    void Clear()
    {
        ClearChildren();
        mWorldViewFrame.ClearChildren();
    }

private:
        void AddLabel(std::string_view text)
    {
        mLabel.SetText(text);
        mLabel.SetCenter(mCenter);
        AddChildBack(&mLabel);
    }

    std::pair<glm::vec2, std::string_view> SetText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredX,
        bool centeredY,
        bool isBold,
        bool isInWorldView)
    {
        Clear();

        switch (dialogFrame)
        {
        case DialogFrame::Fullscreen:
        {
            AddChildBack(&mFullscreenFrame);
            auto [charPos, remaining] = mFullscreenTextBox.SetText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
            return std::make_pair<glm::vec2, std::string_view>(charPos + mFullscreenTextBox.GetTopLeft(), std::move(remaining));
        } break;
        case DialogFrame::ActionAreaInventory: [[fallthrough]];
        case DialogFrame::ActionArea:
        {
            AddChildBack(&mActionAreaFrame);
            if (dialogFrame == DialogFrame::ActionArea)
            {
                mActionAreaFrame.SetPosition({13, 11});
                mActionAreaFrame.SetDimensions({295, 101});
                mActionAreaTextBox.SetDimensions({288, 97});
            }
            // Inventory style is a bit bigger..
            else
            {
                mActionAreaFrame.SetPosition({12, 11});
                mActionAreaFrame.SetDimensions({295, 121});
                mActionAreaTextBox.SetDimensions({288, 118});
            }
            auto [charPos, remaining] = mActionAreaTextBox.SetText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
            return std::make_pair<glm::vec2, std::string_view>(charPos + mActionAreaFrame.GetTopLeft(), std::move(remaining));
        } break;
        case DialogFrame::LowerArea:
        {
            if (isInWorldView)
            {
                AddChildBack(&mWorldViewFrame);
                mWorldViewFrame.AddChildBack(&mLowerFrame);
            }
            else
            {
                AddChildBack(&mLowerFrame);
            }
            auto [charPos, remaining] = mLowerTextBox.SetText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
            // the subtraction of 10 is a hack to stop the buttons getting displayed half off the bottom of the screen... not ideal
            return std::make_pair<glm::vec2, std::string_view>(charPos + mLowerFrame.GetTopLeft() - glm::vec2{0, 10}, std::move(remaining));
        } break;
        case DialogFrame::Popup:
        {
            AddChildBack(&mPopup);
            auto [charPos, remaining] = mPopupText.SetText(mFont, text, true, true);
            // probably not required...
            return std::make_pair<glm::vec2, std::string_view>(charPos + mPopup.GetTopLeft() - glm::vec2{0, 10}, std::move(remaining));
        }   break;
        default:
            throw std::runtime_error("Invalid DialogArea");
        }
    }

    void SetActor(
        unsigned actor,
        IDialogScene& dialogScene,
        bool showName)
    {
        mLogger.Debug() <<" Actor: " << actor << "\n";
        if (actor > 6)
            dialogScene.DisplayNPCBackground();
        else
            dialogScene.DisplayPlayerBackground();

        const auto& [index, dims] = mActors.GetActor(actor);
        mActor.SetTexture(index);
        // we want the bottom of this picture to end up here
        mActor.SetPosition(glm::vec2{100, 112 - dims.y});
        mActor.SetDimensions(dims);
        AddChildBack(&mActor);

        if (showName)
        {
            AddLabel("#" + std::string{mKeywords.GetNPCName(actor)} + "#");
        }
    }

private:
    BAK::Keywords mKeywords;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;
    const Font& mFont;
    const Actors& mActors;
    Label mLabel;
    Widget mActor;

    Widget mFullscreenFrame;
    Widget mWorldViewFrame;
    Widget mActionAreaFrame;
    Widget mActionAreaBackground;
    Widget mLowerFrame;

    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;
    Button mPopup;
    TextBox mPopupText;

    const Logging::Logger& mLogger;
};

}
