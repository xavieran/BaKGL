#pragma once

#include "bak/layout.hpp"

#include "gui/clickButton.hpp"
#include "gui/contents.hpp"
#include "gui/preferencesScreen.hpp"
#include "gui/saveScreen.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

namespace Gui {

class IGuiManager;
class Backgrounds;
class Icons;
class Font;

class MainMenuScreen: public Widget
{
public:
    static constexpr auto sMainMenuSong = 1015;

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
        Graphics::SpriteManager& spriteManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font);

    void EnterMainMenu(bool gameRunning);
    [[nodiscard]] bool OnMouseEvent(const MouseEvent& event) override;
private:
    enum class State
    {
        MainMenu,
        Preferences,
        Save,
        Contents
    };

    void ShowPreferences();
    void ShowContents();
    void ShowSaveOrLoad(bool isSave);
    void BackToMainMenu();
    void EnterMainView();
    void StartNewGame();
    void Load(std::string file);
    void AddChildren();
    void AddMainMenu();
    void AddPreferences();
    void AddContents();
    void AddSave();

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

    PreferencesScreen mPreferencesScreen;
    ContentsScreen mContentsScreen;
    SaveScreen mSaveScreen;

    State mState;
    bool mGameRunning;

    const Logging::Logger& mLogger;
};

}
