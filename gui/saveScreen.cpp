#include "gui/saveScreen.hpp"

#include "bak/saveManager.hpp"

#include "com/path.hpp"

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/list.hpp"
#include "gui/clickButton.hpp"
#include "gui/scrollView.hpp"
#include "gui/textBox.hpp"
#include "gui/textInput.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <filesystem>
#include <functional>

namespace Gui {

SaveScreen::SaveScreen(
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
        [&]{ RemoveDirectory(); }
    },
    mRmFile{
        mLayout.GetWidgetLocation(sRmFile),
        mLayout.GetWidgetDimensions(sRmFile),
        mFont,
        "#Remove File",
        [&]{ RemoveFile(); }
    },
    mSave{
        mLayout.GetWidgetLocation(sSave),
        mLayout.GetWidgetDimensions(sSave),
        mFont,
        "#Save",
        [this]{ SaveGame(false); }
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
    mSelectedDirectory{},
    mSelectedSave{},
    mSaveManager{(Paths::Get().GetBakDirectoryPath() / "GAMES").string()},
    mNeedRefresh{false},
    mLogger{Logging::LogState::GetLogger("Gui::SaveScreen")}
{
    mDirectoryLabel.SetText(mFont, "Directories");
    mFilesLabel.SetText(mFont, "Games");
    mRestoreLabel.SetText(mFont, "#Restore Game");
}

bool SaveScreen::OnMouseEvent(const MouseEvent& event)
{
    const bool handled = Widget::OnMouseEvent(event);

    if (mNeedRefresh)
    {
        RefreshGui();
    }

    return handled;
}

void SaveScreen::SetSaveOrLoad(bool isSave)
{
    mSaveManager.RefreshSaves();

    mIsSave = isSave;
    mDirectories.SetDimensions(glm::vec2{100, mIsSave ? 90 : 108});
    mFiles.SetDimensions(glm::vec2{160, mIsSave ? 90 : 108});

    if (!mSelectedDirectory || !mSelectedSave)
    {
        if (mSaveManager.GetSaves().size() > 0)
        {
            mSelectedDirectory = 0;
            if (mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves.size() > 0)
            {
                mSelectedSave = 0;
            }
        }
    }

    mDirectorySaveInput.SetText(
        mSelectedDirectory
            ? mSaveManager.GetSaves().at(*mSelectedDirectory).mName
            : "");

    mFileSaveInput.SetText(
        (mSelectedDirectory && mSelectedSave)
            ? mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves.at(*mSelectedSave).mName
            : "");

    mRefreshSaves = true;
    mRefreshDirectories = true;
    RefreshGui();
}

void SaveScreen::RemoveDirectory()
{
    if (!mSelectedDirectory)
        return;

    mSaveManager.RemoveDirectory(*mSelectedDirectory);
    if (*mSelectedDirectory > 0)
    {
        (*mSelectedDirectory)--;
    }
    RefreshGui();
}

void SaveScreen::RemoveFile()
{
    if (!mSelectedSave)
        return;

    assert(mSelectedDirectory);

    // Disable remove file button when no saves in selected dir
    mSaveManager.RemoveSave(*mSelectedDirectory, *mSelectedSave);
    if (*mSelectedSave > 0)
    {
        (*mSelectedSave)--;
        mFileSaveInput.SetText(mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves.front().mName);

    }
    if (mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves.size() == 0)
    {
        RemoveDirectory();
    }
    RefreshGui();
}

void SaveScreen::SaveGame(bool isBookmark)
{
    const auto saveDir = mDirectorySaveInput.GetText();
    const auto saveName = mFileSaveInput.GetText();
    if (saveDir.empty() || saveName.empty())
    {
        mLogger.Error() << "Cannot save game, no directory or save name\n";
        return;
    }

    mLogger.Info() << "Saving game to: " << saveDir << " " << saveName << "\n";
    mSaveFn(mSaveManager.MakeSave(saveDir, saveName, isBookmark));
}

void SaveScreen::RestoreGame()
{
    assert(mSelectedDirectory && mSelectedSave);
    const auto savePath = mSaveManager.GetSaves().at(*mSelectedDirectory)
        .mSaves.at(*mSelectedSave).mPath;
    std::invoke(mLoadSaveFn, savePath);
}

void SaveScreen::DirectorySelected(std::size_t i)
{
    mSelectedDirectory = i;
    const auto& saves = mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves;
    mSelectedSave = saves.size() > 0
        ? std::make_optional(0)
        : std::nullopt;

    mDirectorySaveInput.SetFocus(true);
    mFileSaveInput.SetFocus(false);
    mDirectorySaveInput.SetText(
        mSaveManager.GetSaves().at(*mSelectedDirectory).mName);
    mFileSaveInput.SetText(saves.size() > 0
            ? saves.front().mName
            : "");

    mNeedRefresh = true;
    mRefreshSaves = true;
}

void SaveScreen::SaveSelected(std::size_t i)
{
    assert(mSelectedDirectory);
    const auto& saves = mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves;

    mSelectedSave = i;
    mDirectorySaveInput.SetFocus(false);
    mFileSaveInput.SetFocus(true);
    const auto saveName = saves.at(*mSelectedSave).mName;
    mFileSaveInput.SetText(saveName);

    mNeedRefresh = true;
}

void SaveScreen::RefreshGui()
{
    mNeedRefresh = false;
    AddChildren();
}

void SaveScreen::AddChildren()
{
    mDirectories.GetChild().ClearWidgets();
    mFiles.GetChild().ClearWidgets();
    ClearChildren();

    AddChildBack(&mFrame);

    std::size_t index = 0;
    for (const auto& dir : mSaveManager.GetSaves())
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

    if (mSelectedDirectory)
    {
        index = 0;
        for (auto save : mSaveManager.GetSaves().at(*mSelectedDirectory).mSaves)
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

}
