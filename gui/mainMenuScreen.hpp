#pragma once

#include "audio/audio.hpp"

#include "bak/IContainer.hpp"
#include "bak/textureFactory.hpp"


#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class MainMenuScreen: public Widget
{
public:
    static constexpr auto sMainMenuSong = AudioA::MusicIndex{1015};

    static constexpr auto sLayoutFile = "REQ_OPT0.DAT";
    static constexpr auto sLayoutFileGameRunning = "REQ_OPT1.DAT";
    static constexpr auto sBackground = "OPTIONS0.SCX";
    static constexpr auto sBackgroundGameRunning = "OPTIONS1.SCX";

    static constexpr auto sStartNew = 0;
    static constexpr auto sRestore = 1;
    static constexpr auto sSaveGame= 2;
    static constexpr auto sPreferences = 3;
    static constexpr auto sContents = 4;
    static constexpr auto sQuit = 5;
    static constexpr auto sCancel = 6;

    MainMenuScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Font& font)
    :
        Widget{
            RectTag{},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            Color::black,
            false 
        },
        mGuiManager{guiManager},
        mFont{font},
        mBackgrounds{backgrounds},
        mLayout{sLayoutFile},
        mLayoutGameRunning{sLayoutFileGameRunning},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
        },
        mStartNew{
            mLayout.GetWidgetLocation(sStartNew),
            mLayout.GetWidgetDimensions(sStartNew),
            mFont,
            "#Start New Game",
            []{ }
        },
        mRestore{
            mLayout.GetWidgetLocation(sRestore),
            mLayout.GetWidgetDimensions(sRestore),
            mFont,
            "#Restore Game",
            []{ }
        },
        mSaveGame{
            mLayoutGameRunning.GetWidgetLocation(sSaveGame),
            mLayoutGameRunning.GetWidgetDimensions(sSaveGame),
            mFont,
            "#Save Game",
            []{ }
        },
        mPreferences{
            mLayout.GetWidgetLocation(sPreferences),
            mLayout.GetWidgetDimensions(sPreferences),
            mFont,
            "#Preferences",
            []{ }
        },
        mContents{
            mLayout.GetWidgetLocation(sContents),
            mLayout.GetWidgetDimensions(sContents),
            mFont,
            "#Contents",
            []{ }
        },
        mQuit{
            mLayout.GetWidgetLocation(sQuit),
            mLayout.GetWidgetDimensions(sQuit),
            mFont,
            "#Quit to DOS",
            []{ std::exit(0); }
        },
        mCancel{
            mLayoutGameRunning.GetWidgetLocation(sCancel),
            mLayoutGameRunning.GetWidgetDimensions(sCancel),
            mFont,
            "#Cancel",
            []{ }
        },
        mGameRunning{false},
        mLogger{Logging::LogState::GetLogger("Gui::MainMenuScreen")}
    {
    }

    void EnterMainMenu(bool gameRunning)
    {
        mGameRunning = gameRunning;

        AddChildren();

        AudioA::AudioManager::Get().ChangeMusicTrack(sMainMenuSong);
    }

    void ExitMainMenu()
    {
        AudioA::AudioManager::Get().PopTrack();
    }

private:
    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);

        if (mGameRunning)
        {
            mFrame.SetTexture(mBackgrounds.GetScreen(sBackgroundGameRunning));

            mStartNew.SetPosition(mLayoutGameRunning.GetWidgetLocation(sStartNew));
            mRestore.SetPosition(mLayoutGameRunning.GetWidgetLocation(sRestore));
            mPreferences.SetPosition(mLayoutGameRunning.GetWidgetLocation(sPreferences));
            mContents.SetPosition(mLayoutGameRunning.GetWidgetLocation(sContents));
            mQuit.SetPosition(mLayoutGameRunning.GetWidgetLocation(sQuit));

            AddChildBack(&mSaveGame);
            AddChildBack(&mCancel);
        }
        else
        {
            mFrame.SetTexture(mBackgrounds.GetScreen(sBackground));

            mStartNew.SetPosition(mLayout.GetWidgetLocation(sStartNew));
            mRestore.SetPosition(mLayout.GetWidgetLocation(sRestore));
            mPreferences.SetPosition(mLayout.GetWidgetLocation(sPreferences));
            mContents.SetPosition(mLayout.GetWidgetLocation(sContents));
            mQuit.SetPosition(mLayout.GetWidgetLocation(sQuit));
        }

        AddChildBack(&mStartNew);
        AddChildBack(&mRestore);
        AddChildBack(&mPreferences);
        AddChildBack(&mContents);
        AddChildBack(&mQuit);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    BAK::Layout mLayoutGameRunning;

    Widget mFrame;
    ClickButton mStartNew;
    ClickButton mRestore;
    ClickButton mSaveGame;
    ClickButton mPreferences;
    ClickButton mContents;
    ClickButton mQuit;
    ClickButton mCancel;

    bool mGameRunning;

    const Logging::Logger& mLogger;
};

}
