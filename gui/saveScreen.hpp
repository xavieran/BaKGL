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

class SaveScreen: public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_SAVE.DAT";
    static constexpr auto sBackground = "OPTIONS2.SCX";

    static constexpr auto sRmDirectory = 2;
    static constexpr auto sRmFile = 3;
    static constexpr auto sSave = 4;
    static constexpr auto sCancel = 5;

    using LeaveSaveFn = std::function<void()>;

    SaveScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Font& font,
        LeaveSaveFn&& leaveSaveFn)
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
        mLeaveSaveFn{std::move(leaveSaveFn)},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
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
            []{ }
        },
        mCancel{
            mLayout.GetWidgetLocation(sCancel),
            mLayout.GetWidgetDimensions(sCancel),
            mFont,
            "#Cancel",
            [this]{ std::invoke(mLeaveSaveFn); }
        },
        mLogger{Logging::LogState::GetLogger("Gui::SaveScreen")}
    {
        AddChildren();
    }

private:
    void AddChildren()
    {
        ClearChildren();

        AddChildBack(&mFrame);

        mRmDirectory.SetPosition(mLayout.GetWidgetLocation(sRmDirectory));
        mRmFile.SetPosition(mLayout.GetWidgetLocation(sRmFile));
        mSave.SetPosition(mLayout.GetWidgetLocation(sSave));
        mCancel.SetPosition(mLayout.GetWidgetLocation(sCancel));

        AddChildBack(&mRmDirectory);
        AddChildBack(&mRmFile);
        AddChildBack(&mSave);
        AddChildBack(&mCancel);
    }

    IGuiManager& mGuiManager;
    const Font& mFont;
    const Backgrounds& mBackgrounds;

    BAK::Layout mLayout;
    LeaveSaveFn mLeaveSaveFn;

    Widget mFrame;
    ClickButton mRmDirectory;
    ClickButton mRmFile;
    ClickButton mSave;
    ClickButton mCancel;

    const Logging::Logger& mLogger;
};

}
