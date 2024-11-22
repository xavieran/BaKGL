#pragma once

#include "bak/layout.hpp"
#include "bak/saveManager.hpp"

#include "gui/list.hpp"
#include "gui/clickButton.hpp"
#include "gui/scrollView.hpp"
#include "gui/textBox.hpp"
#include "gui/textInput.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <functional>

namespace Gui {

class Backgrounds;
class Icons;
class Font;

class SaveScreen: public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_SAVE.DAT";
    static constexpr auto sLayoutRestoreFile = "REQ_LOAD.DAT";
    static constexpr auto sBackground = "OPTIONS2.SCX";

    static constexpr auto sDirectories = 0;
    static constexpr auto sFiles = 1;
    static constexpr auto sRmDirectory = 2;
    static constexpr auto sRmFile = 3;
    static constexpr auto sSave = 4;
    static constexpr auto sCancel = 5;
    static constexpr auto sLoadOffset = 2;

    using LeaveSaveFn = std::function<void()>;
    using LoadSaveFn = std::function<void(std::string)>;
    using SaveFn = std::function<void(const BAK::SaveFile&)>;

    SaveScreen(
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        LeaveSaveFn&& leaveSaveFn,
        LoadSaveFn&& loadSaveFn,
        SaveFn&& saveFn);
    
    bool OnMouseEvent(const MouseEvent& event) override;
    void SetSaveOrLoad(bool isSave);
private:
    void RemoveDirectory();
    void RemoveFile();
    void SaveGame(bool isBookmark);
    void RestoreGame();
    void DirectorySelected(std::size_t i);
    void SaveSelected(std::size_t i);
    void RefreshGui();
    void AddChildren();

    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    BAK::Layout mLayoutRestore;
    LeaveSaveFn mLeaveSaveFn;
    LoadSaveFn mLoadSaveFn;
    SaveFn mSaveFn;

    bool mIsSave;
    Widget mFrame;
    TextBox mRestoreLabel;
    TextBox mDirectoryLabel;
    TextBox mFilesLabel;

    ScrollView<List<ClickButton>> mDirectories;
    TextInput mDirectorySaveInput;

    ScrollView<List<ClickButton>> mFiles;
    TextInput mFileSaveInput;

    ClickButton mRmDirectory;
    ClickButton mRmFile;
    ClickButton mSave;
    ClickButton mRestore;
    ClickButton mCancel;

    bool mRefreshDirectories;
    bool mRefreshSaves;
    std::optional<std::size_t> mSelectedDirectory;
    std::optional<std::size_t> mSelectedSave;
    BAK::SaveManager mSaveManager;

    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
