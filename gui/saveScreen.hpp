#pragma once

#include "bak/saveFile.hpp"

#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/list.hpp"
#include "gui/clickButton.hpp"
#include "gui/scrollView.hpp"
#include "gui/textBox.hpp"
#include "gui/widget.hpp"

#include <glm/glm.hpp>

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

    SaveScreen(
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& font,
        LeaveSaveFn&& leaveSaveFn,
        LoadSaveFn&& loadSaveFn)
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
        mIsSave{true},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
        },
        mDirectoryLabel{
            glm::vec2{22, 20},
            glm::vec2{80, 16}
        },
        mFilesLabel{
            glm::vec2{132, 20},
            glm::vec2{40, 16}
        },
        mDirectories{
            glm::vec2{20, 30},
            glm::vec2{100, 110},
            icons,
            false,
            true,
            40u, 0.0f, true},
        mFiles{
            glm::vec2{130, 30},
            glm::vec2{160, 110},
            icons,
            false,
            true,
            40u, 0.0f, true},
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
            []{ }
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
        mSelectedDirectory{0},
        mSelectedSave{0},
        mSaveDirs{
            BAK::MakeSaveDirectories()
        },
        mNeedRefresh{false},
        mLogger{Logging::LogState::GetLogger("Gui::SaveScreen")}
    {
        mDirectoryLabel.AddText(mFont, "Directories");
        mFilesLabel.AddText(mFont, "Files");
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
        RefreshGui();
    }

private:
    void RestoreGame()
    {
        const auto savePath = mSaveDirs.at(mSelectedDirectory).mSaves.at(mSelectedSave).mPath;
        std::invoke(mLoadSaveFn, savePath);
    }

    void DirectorySelected(std::size_t i)
    {
        mSelectedDirectory = i;
        mSelectedSave = 0;
        mNeedRefresh = true;
    }

    void SaveSelected(std::size_t i)
    {
        mSelectedSave = i;
        mNeedRefresh = true;
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
        mDirectories.ResetScroll();

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
        mFiles.ResetScroll();

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

            AddChildBack(&mDirectories);
            AddChildBack(&mRmDirectory);
            AddChildBack(&mRmFile);
            AddChildBack(&mSave);
        }
        else
        {
            mCancel.SetPosition(mLayoutRestore.GetWidgetLocation(sCancel - sLoadOffset));
            AddChildBack(&mRestore);
        }
    }

    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    BAK::Layout mLayoutRestore;
    LeaveSaveFn mLeaveSaveFn;
    LoadSaveFn mLoadSaveFn;

    bool mIsSave;
    Widget mFrame;
    TextBox mDirectoryLabel;
    TextBox mFilesLabel;
    ScrollView<List<ClickButton>> mDirectories;
    ScrollView<List<ClickButton>> mFiles;
    ClickButton mRmDirectory;
    ClickButton mRmFile;
    ClickButton mSave;
    ClickButton mRestore;
    ClickButton mCancel;

    std::size_t mSelectedDirectory;
    std::size_t mSelectedSave;
    std::vector<BAK::SaveDirectory> mSaveDirs;

    bool mNeedRefresh;

    const Logging::Logger& mLogger;
};

}
