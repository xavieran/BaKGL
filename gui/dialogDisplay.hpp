#pragma once

#include "bak/dialog.hpp"
#include "bak/gameState.hpp"

#include "com/algorithm.hpp"
#include "com/visit.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/actors.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/frame.hpp"
#include "gui/label.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <string>
#include <string_view>

namespace Gui {

enum class DialogFrame
{
    Fullscreen = 0,
    ActionArea = 1,
    LowerArea = 2
};

class DialogDisplay : public Widget
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
        mDialogStore{},
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
        mActionAreaFrame{
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
            glm::vec2{15, 125},
            glm::vec2{285, 66}
        },
        mFullscreenTextBox{
            glm::vec2{30, 30},
            glm::vec2{320 - 30*2, 135}
        },
        mActionAreaTextBox{
            glm::vec2{4, 4},
            glm::vec2{290, 103}
        },
        mLowerTextBox{
            glm::vec2{0, 0},
            glm::vec2{285, 66}
        },
        mLogger{Logging::LogState::GetLogger("Gui::DialogDisplay")}
    {
        mFullscreenFrame.AddChildBack(&mFullscreenTextBox);

        mLowerFrame.AddChildBack(&mLowerTextBox);

        mActionAreaFrame.AddChildBack(&mActionAreaBackground);
        mActionAreaFrame.AddChildBack(&mActionAreaTextBox);
    }

    void Clear()
    {
        ClearChildren();
    }

    void DisplayPlayer(IDialogScene& dialogScene, unsigned act)
    {
        const auto actor = mGameState.GetActor(act);
        SetActor(actor, dialogScene, false);
        AddLabel("#" 
            + std::string{mKeywords.GetNPCName(actor)}
            + " asked about:#");
    }

    auto DisplaySnippet(
        IDialogScene& dialogScene,
        const BAK::DialogSnippet& snippet,
        std::optional<std::string_view> remainingText)
    {
        ClearChildren();

        std::string text{};
        if (remainingText)
            text = std::string{*remainingText};
        else
            text = std::string{snippet.GetText()};

        text = mGameState.GetTextVariableStore().SubstituteVariables(text);

        const auto ds1 = snippet.mDisplayStyle;
        const auto act = snippet.mActor;

        const auto dialogFrame = std::invoke([ds1, act]{
            if (act != 0x0)
                return DialogFrame::LowerArea;

            if (ds1 == 0x02)
                return DialogFrame::ActionArea;
            else if (ds1 == 0x03
                || ds1 == 0x04)
                return DialogFrame::LowerArea;
            else
                return DialogFrame::Fullscreen;
        });

        const auto ds2 = snippet.mDisplayStyle2;
        const bool verticallyCentered
            = ((ds2 & 0x10) == 0x10) || (ds2 == 0x3);
        const bool horizontallyCentered 
            = (ds2 & 0x4) == 0x4;
        const bool isBold
            = ds2 == 0x3;

        std::optional<unsigned> actor{};

        if (act != 0)
        {
            actor = mGameState.GetActor(act);
        }

        const auto [charPos, undisplayedText] = AddText(
            text,
            dialogFrame,
            horizontallyCentered,
            verticallyCentered,
            isBold);

        if (actor)
        {
            SetActor(*actor, dialogScene, true);
        }

        return std::string{undisplayedText};
    }

    void ShowFlavourText(BAK::Target target)
    {
        const auto& snippet = mDialogStore.GetSnippet(target);
        const auto text = snippet.GetText();
        const auto flavourText = find_nth(text.begin(), text.end(), '#', 2);
        assert(flavourText != text.end());
        const auto remainingText = std::string{flavourText, text.end()};
        auto nullScene = NullDialogScene{};
        DisplaySnippet(nullScene, snippet, remainingText);

        const auto label = std::string{text.begin(), flavourText};
        AddLabel(label);
    }

private:
    void AddLabel(std::string_view text)
    {
        mLabel.SetText(text);
        mLabel.SetCenter(mCenter);
        AddChildBack(&mLabel);
    }

    std::pair<glm::vec2, std::string_view> AddText(
        std::string_view text,
        DialogFrame dialogFrame,
        bool centeredX,
        bool centeredY,
        bool isBold)
    {
        ClearChildren();

        switch (dialogFrame)
        {
        case DialogFrame::Fullscreen:
        {
            AddChildBack(&mFullscreenFrame);
            return mFullscreenTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
        } break;
        case DialogFrame::ActionArea:
        {
            AddChildBack(&mActionAreaFrame);
            return mActionAreaTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
        } break;
        case DialogFrame::LowerArea:
        {
            AddChildBack(&mLowerFrame);
            return mLowerTextBox.AddText(
                mFont,
                text,
                centeredX,
                centeredY,
                isBold);
        } break;
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
    BAK::DialogStore mDialogStore;
    BAK::Keywords mKeywords;
    BAK::GameState& mGameState;
    glm::vec2 mCenter;
    const Font& mFont;
    const Actors& mActors;
    Label mLabel;
    Widget mActor;

    Widget mFullscreenFrame;
    Frame mActionAreaFrame;
    Widget mActionAreaBackground;
    Frame mLowerFrame;

    TextBox mFullscreenTextBox;
    TextBox mActionAreaTextBox;
    TextBox mLowerTextBox;

    const Logging::Logger& mLogger;
};

}
