#pragma once

#include "bak/saveFile.hpp"

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/list.hpp"
#include "gui/clickButton.hpp"
#include "gui/scrollView.hpp"
#include "gui/textBox.hpp"
#include "gui/textInput.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <iomanip>
#include <filesystem>
#include <functional>

namespace Gui {

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
        SaveFn&& saveFn)
    :
        Widget{
            RectTag{},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            Color::black,
            false 
        },
        mFont{font},
        mBackgrounds{backgrounds},
        mLayout{sLayoutFile},
        mLayoutRestore{sLayoutRestoreFile},
        mLeaveSaveFn{std::move(leaveSaveFn)},
        mLoadSaveFn{std::move(loadSaveFn)},
        mSaveFn{std::move(saveFn)},
        mIsSave{true},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
        },
        mRestoreLabel{
            glm::vec2{132, 12},
            glm::vec2{80, 16}
        },
        mDirectoryLabel{
            glm::vec2{22, 30},
            glm::vec2{80, 16}
        },
        mFilesLabel{
            glm::vec2{132, 30},
            glm::vec2{40, 16}
        },
        mDirectories{
            glm::vec2{20, 40},
            glm::vec2{100, 90},
            icons,
            false,
            true,
            40u, 0.0f, true},
        mDirectorySaveInput{
            font,
            glm::vec2{20, 132},
            glm::vec2{100, 16},
            30
        },
        mFiles{
            glm::vec2{130, 40},
            glm::vec2{160, 90},
            icons,
            false,
            true,
            40u, 0.0f, true},
        mFileSaveInput{
            font,
            glm::vec2{130, 132},
            glm::vec2{160, 16},
            30
        },
        mRmDirectory{
            mLayout.GetWidgetLocation(sRmDirectory),
            mLayout.GetWidgetDimensions(sRmDirectory),
            mFont,
            "#Remove Directory",
            []{ }
        },
        mRmFile{
            mLayout.GetWidgetLocation(sRmFile),
            mLayout.GetWidgetDimensions(sRmFile),
            mFont,
            "#Remove File",
            []{ }
        },
        mSave{
            mLayout.GetWidgetLocation(sSave),
            mLayout.GetWidgetDimensions(sSave),
            mFont,
            "#Save",
            [this]{ SaveGame(); }
        },
        mRestore{
            mLayoutRestore.GetWidgetLocation(sSave - sLoadOffset),
            mLayoutRestore.GetWidgetDimensions(sSave - sLoadOffset),
            mFont,
            "#Restore",
            [this]{ RestoreGame(); }
        },
        mCancel{
            mLayout.GetWidgetLocation(sCancel),
            mLayout.GetWidgetDimensions(sCancel),
            mFont,
            "#Cancel",
            [this]{ std::invoke(mLeaveSaveFn); }
        },
        mRefreshDirectories{false},
        mRefreshSaves{false},
        mSelectedDirectory{0},
        mSelectedSave{0},
        mSaveDirs{
            BAK::MakeSaveDirectories()
        },
        mNeedRefresh{false},
        mLogger{Logging::LogState::GetLogger("Gui::SaveScreen")}
    {
        mDirectoryLabel.AddText(mFont, "Directories");
        mFilesLabel.AddText(mFont, "Games");
        mRestoreLabel.AddText(mFont, "#Restore Game");
    }

    bool OnMouseEvent(const MouseEvent& event) override
    {
        const bool handled = Widget::OnMouseEvent(event);

        if (mNeedRefresh)
        {
            RefreshGui();
        }

        return handled;
    }

    void SetSaveOrLoad(bool isSave)
    {
        mIsSave = isSave;
        mRefreshSaves = true;
        mRefreshDirectories = true;
        RefreshGui();
    }

private:
    void SaveGame()
    {
        const auto saveDir = mDirectorySaveInput.GetText();
        const auto saveName = mFileSaveInput.GetText();

        const auto selectedDirectoryName = std::filesystem::path{mSaveDirs.at(mSelectedDirectory).mName}.stem();

        mLogger.Debug() << "Selected save dir: " << selectedDirectoryName
            << " :: " << mSaveDirs.at(mSelectedDirectory).mSaves.at(mSelectedSave).mName << "\n";

        if (saveDir == selectedDirectoryName)
        {
            if (saveName == mSaveDirs.at(mSelectedDirectory).mSaves.at(mSelectedSave).mName)
            {
                mLogger.Debug() << "Overwrite: " << mSaveDirs.at(mSelectedDirectory).mSaves.at(mSelectedSave).mPath << "\n";
            }
            else
            {
                const auto saveDirPath = mSaveDirs.at(mSelectedDirectory);
                mLogger.Debug() << "Save dir: " << saveDirPath.mName<< "\n";
                mLogger.Debug() << "File name: SAVE" << std::setw(2) << std::setfill('0') << saveDirPath.mSaves.size() << ".GAM -> " << saveName << "\n";
            }
        }
        else
        {
                mLogger.Debug() << "New dir: " << saveDir << std::setw(2) << std::setfill('0')<< ".G" << mSaveDirs.size() << "\n";
                mLogger.Debug() << "File name: SAVE01.GAM -> " << saveName << "\n";
        }
    }

    void RestoreGame()
    {
        const auto savePath = mSaveDirs.at(mSelectedDirectory)
            .mSaves.at(mSelectedSave).mPath;
        std::invoke(mLoadSaveFn, savePath);
    }

    void DirectorySelected(std::size_t i)
    {
        mSelectedDirectory = i;
        mSelectedSave = 0;
        mNeedRefresh = true;
        mRefreshSaves = true;

        mDirectorySaveInput.SetFocus(true);
        mDirectorySaveInput.SetText(mSaveDirs.at(mSelectedDirectory).mName);
        mFileSaveInput.SetFocus(false);
    }

    void SaveSelected(std::size_t i)
    {
        mSelectedSave = i;
        mNeedRefresh = true;

        mDirectorySaveInput.SetFocus(false);
        mFileSaveInput.SetFocus(true);
        const auto saveName = mSaveDirs.at(mSelectedDirectory)
            .mSaves.at(mSelectedSave).mName;
        mFileSaveInput.SetText(saveName);
    }

    void RefreshGui()
    {
        mNeedRefresh = false;
        AddChildren();
    }

    void AddChildren()
    {
        mDirectories.GetChild().ClearWidgets();
        mFiles.GetChild().ClearWidgets();
        ClearChildren();

        AddChildBack(&mFrame);

        std::size_t index = 0;
        for (const auto& dir : mSaveDirs)
        {
            mDirectories.GetChild().AddWidget(
                glm::vec2{0, 0},
                glm::vec2{mDirectories.GetDimensions().x - 16, 15},
                mFont,
                (index == mSelectedDirectory ? "#" : "") + dir.mName,
                [this, i=index]{ DirectorySelected(i); }
            );
            index++;
        }

        index = 0;
        for (auto save : mSaveDirs.at(mSelectedDirectory).mSaves)
        {
            mFiles.GetChild().AddWidget(
                glm::vec2{0, 0},
                glm::vec2{mFiles.GetDimensions().x - 16, 15},
                mFont,
                (index == mSelectedSave ? "#" : "") + save.mName,
                [this, i=index]{ SaveSelected(i); }
            );
            index++;
        }

        AddChildBack(&mDirectories);
        AddChildBack(&mFiles);
        AddChildBack(&mDirectoryLabel);
        AddChildBack(&mFilesLabel);
        AddChildBack(&mCancel);

        if (mIsSave)
        {
            mRmDirectory.SetPosition(mLayout.GetWidgetLocation(sRmDirectory));
            mRmFile.SetPosition(mLayout.GetWidgetLocation(sRmFile));
            mSave.SetPosition(mLayout.GetWidgetLocation(sSave));
            mCancel.SetPosition(mLayout.GetWidgetLocation(sCancel));

            AddChildBack(&mRmDirectory);
            AddChildBack(&mRmFile);
            AddChildBack(&mSave);
            AddChildBack(&mRestoreLabel);
            AddChildBack(&mDirectorySaveInput);
            AddChildBack(&mFileSaveInput);
        }
        else
        {
            mCancel.SetPosition(mLayoutRestore.GetWidgetLocation(sCancel - sLoadOffset));
            AddChildBack(&mRestore);
            AddChildBack(&mRestoreLabel);
        }

        // Only refresh this when the directory changes
        // otherwises the scroll view will pop back to the top
        // when we click on a save
        if (mRefreshSaves)
        {
            mFiles.ResetScroll();
            mRefreshSaves = false;
        }
        if (mRefreshDirectories)
        {
            mDirectories.ResetScroll();
            mRefreshDirectories = false;
        }
    }

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
    std::size_t mSelectedDirectory;
    std::size_t mSelectedSave;
    std::vector<BAK::SaveDirectory> mSaveDirs;

    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
