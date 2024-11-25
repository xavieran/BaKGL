#include "gui/mainMenuScreen.hpp"

#include "audio/audio.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

MainMenuScreen::MainMenuScreen(
    IGuiManager& guiManager,
    Graphics::SpriteManager& spriteManager,
    const Backgrounds& backgrounds,
    const Icons& icons,
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
        [this]{ StartNewGame(); }
    },
    mRestore{
        mLayout.GetWidgetLocation(sRestore),
        mLayout.GetWidgetDimensions(sRestore),
        mFont,
        "#Restore Game",
        [this]{ ShowSaveOrLoad(false); }
    },
    mSaveGame{
        mLayoutGameRunning.GetWidgetLocation(sSaveGame),
        mLayoutGameRunning.GetWidgetDimensions(sSaveGame),
        mFont,
        "#Save Game",
        [this]{ ShowSaveOrLoad(true); }
    },
    mPreferences{
        mLayout.GetWidgetLocation(sPreferences),
        mLayout.GetWidgetDimensions(sPreferences),
        mFont,
        "#Preferences",
        [this]{ ShowPreferences(); }
    },
    mContents{
        mLayout.GetWidgetLocation(sContents),
        mLayout.GetWidgetDimensions(sContents),
        mFont,
        "#Contents",
        [this]{ ShowContents(); }
    },
    mQuit{
        mLayout.GetWidgetLocation(sQuit),
        mLayout.GetWidgetDimensions(sQuit),
        mFont,
        "#Quit to DOS",
        [this]{ 
            mGuiManager.DoFade(
                1.0,
                []{ std::exit(0); });
        }
    },
    mCancel{
        mLayoutGameRunning.GetWidgetLocation(sCancel),
        mLayoutGameRunning.GetWidgetDimensions(sCancel),
        mFont,
        "#Cancel",
        [this]{ EnterMainView(); }
    },
    mPreferencesScreen{
        guiManager,
        backgrounds,
        font,
        [this]{ BackToMainMenu(); }
    },
    mContentsScreen{
        guiManager,
        spriteManager,
        backgrounds,
        font,
        [this]{ BackToMainMenu(); }
    },
    mSaveScreen{
        backgrounds,
        icons,
        font,
        [this]{ BackToMainMenu(); },
        [this](const auto& file){
            Load(file);
        },
        [this](const auto& saveFile){
            mState = State::MainMenu;
            AudioA::GetAudioManager().PopTrack();
            mGuiManager.SaveGame(saveFile);
        }
    },
    mState{State::MainMenu},
    mGameRunning{false},
    mLogger{Logging::LogState::GetLogger("Gui::MainMenuScreen")}
{
}

void MainMenuScreen::EnterMainMenu(bool gameRunning)
{
    mGameRunning = gameRunning;

    AddChildren();

    AudioA::GetAudioManager().ChangeMusicTrack(AudioA::MusicIndex{sMainMenuSong});
}

[[nodiscard]] bool MainMenuScreen::OnMouseEvent(const MouseEvent& event)
{
    return Widget::OnMouseEvent(event) || true;
}

void MainMenuScreen::ShowPreferences()
{
    mGuiManager.DoFade(1.0, [this]{
        mState = State::Preferences;
        AddChildren();
    });
}

void MainMenuScreen::ShowContents()
{
    mGuiManager.DoFade(1.0, [this]{
        mState = State::Contents;
        AddChildren();
    });
}

void MainMenuScreen::ShowSaveOrLoad(bool isSave)
{
    mGuiManager.DoFade(1.0, [this, isSave]{
        mState = State::Save;
        mSaveScreen.SetSaveOrLoad(isSave);
        AddChildren();
    });
}

void MainMenuScreen::BackToMainMenu()
{
    mGuiManager.DoFade(1.0, [this]{
        mState = State::MainMenu;
        AddChildren();
    });
}

void MainMenuScreen::EnterMainView()
{
    AudioA::GetAudioManager().PopTrack();
    mGuiManager.EnterMainView();
}

void MainMenuScreen::StartNewGame()
{
    AudioA::GetAudioManager().PopTrack();
    auto start = BAK::CutsceneList::GetStartScene(BAK::Chapter{1});
    mGuiManager.PlayCutscene(start , [&]{ 
        mGuiManager.ShowGameStartMap();
    });
    mGuiManager.LoadGame("startup.gam", std::make_optional(BAK::Chapter{1}));
}

void MainMenuScreen::Load(std::string file)
{
    mState = State::MainMenu;
    AudioA::GetAudioManager().PopTrack();
    mGuiManager.LoadGame(file, std::nullopt);
    mGuiManager.ShowGameStartMap();
}

void MainMenuScreen::AddChildren()
{
    ClearChildren();

    switch (mState)
    {
        case State::MainMenu:
            AddMainMenu();
            break;
        case State::Preferences:
            AddPreferences();
            break;
        case State::Contents:
            AddContents();
            break;
        case State::Save:
            AddSave();
            break;
    }
}

void MainMenuScreen::AddMainMenu()
{
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

void MainMenuScreen::AddPreferences()
{
    AddChildBack(&mPreferencesScreen);
}

void MainMenuScreen::AddContents()
{
    AddChildBack(&mContentsScreen);
}

void MainMenuScreen::AddSave()
{
    AddChildBack(&mSaveScreen);
}

}
