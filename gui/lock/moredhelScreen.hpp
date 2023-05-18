#pragma once

#include "bak/lock.hpp"
#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"
#include "bak/layout.hpp"

#include "gui/lock/tumbler.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/core/clickable.hpp"
#include "gui/core/dragEndpoint.hpp"
#include "gui/core/draggable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class MoredhelScreen :
    public Widget
{
public:
    static constexpr auto sLayoutFile = "REQ_PUZL.DAT";
    static constexpr auto sBackground = "PUZZLE.SCX";

    // Request offsets
    static constexpr auto mExitRequest = 0;
    static constexpr auto mExitButton = 13;

    MoredhelScreen(
        IGuiManager& guiManager,
        const Backgrounds& backgrounds,
        const Icons& icons,
        const Font& alienFont,
        const Font& puzzleFont,
        BAK::GameState& gameState)
    :
        // Black background
        Widget{
            RectTag{},
            glm::vec2{0, 0},
            glm::vec2{320, 200},
            Color::black,
            true
        },
        mGuiManager{guiManager},
        mAlienFont{alienFont},
        mPuzzleFont{puzzleFont},
        mIcons{icons},
        mGameState{gameState},
        mDialogScene{
            []{},
            []{},
            [](const auto&){ }
        },
        mFairyChest{},
        mLayout{sLayoutFile},
        mFrame{
            ImageTag{},
            backgrounds.GetSpriteSheet(),
            backgrounds.GetScreen(sBackground),
            glm::vec2{0},
            GetPositionInfo().mDimensions,
            true
        },
        mExit{
            mLayout.GetWidgetLocation(mExitRequest),
            mLayout.GetWidgetDimensions(mExitRequest),
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetButton(mExitButton)),
            std::get<Graphics::TextureIndex>(mIcons.GetButton(mExitButton)),
            std::get<Graphics::TextureIndex>(mIcons.GetPressedButton(mExitButton)),
            [this]{ CantOpenLock(); },
            []{}
        },
        mLeftClasp{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            mLayout.GetWidgetLocation(1),
            mLayout.GetWidgetDimensions(1),
            true
        },
        mRightClasp{
            ImageTag{},
            std::get<Graphics::SpriteSheetIndex>(mIcons.GetInventoryMiscIcon(11)),
            std::get<Graphics::TextureIndex>(mIcons.GetInventoryMiscIcon(11)),
            mLayout.GetWidgetLocation(1),
            mLayout.GetWidgetDimensions(1),
            true
        },
        mDescription{
            {40, 100},
            {240, 64}
        },
        mTumblers{},
        mReqLocs{},
        mContainer{nullptr},
        mNeedRefresh{false},
        mUnlocked{false},
        mLogger{Logging::LogState::GetLogger("Gui::MoredhelScreen")}
    {
    }

    void ResetUnlocked()
    {
        mUnlocked = false;
    }

    bool IsUnlocked() const
    {
        return mUnlocked;
    }

    BAK::IContainer* GetContainer() const
    {
        return mContainer;
    }

    void SetContainer(BAK::IContainer* container)
    {
        ASSERT(container != nullptr);

        mContainer = container;
        const auto& snippet = BAK::DialogSources::GetFairyChestKey(
            container->GetLock().mFairyChestIndex);
        mFairyChest = BAK::GenerateFairyChest(
            std::string{BAK::DialogStore::Get().GetSnippet(snippet).GetText()});

        ResetUnlocked();

        RefreshGui();
    }

private:
    void RefreshGui()
    {
        ClearChildren();

        UpdateTumblers();

        mDescription.AddText(
            mPuzzleFont, 
            "\xf7" + mFairyChest->mHint, true, true);

        mReqLocs.clear();

        mReqLocs.reserve(mLayout.GetSize());
        for (unsigned i = 0; i < mLayout.GetSize(); i++)
        {
            mReqLocs.emplace_back(
                mLayout.GetWidgetLocation(i),
                mLayout.GetWidgetDimensions(i));
            std::stringstream ss{};
            ss << i;
            mReqLocs.back().AddText(mPuzzleFont, ss.str());
        }

        AddChildren();
    }

    void UpdateTumblers()
    {
        ASSERT(mFairyChest);
        mTumblers.clear();
        const auto tumblers = mFairyChest->mAnswer.size();
        ASSERT(tumblers <= 15);
        unsigned margin = 15 - tumblers;
        unsigned startLocation = margin / 2;
        mTumblers.reserve(tumblers);
        for (unsigned i = 0; i < tumblers; i++)
        {
            mTumblers.emplace_back(
                [this, i](){ IncrementTumbler(i); },
                mLayout.GetWidgetLocation(1 + startLocation + i)
                    + glm::vec2{0, 56},
                mLayout.GetWidgetDimensions(1 + startLocation + i),
                mPuzzleFont);
            mTumblers.back().SetDigits(i, *mFairyChest);
            // if char ==  ' ' dont add the tumbler...?
        }
    }

    void IncrementTumbler(unsigned tumblerIndex)
    {
        ASSERT(tumblerIndex < mTumblers.size());
        mTumblers[tumblerIndex].NextDigit();

        EvaluateLock();
    }

    void EvaluateLock()
    {
        std::string guess{};
        for (const auto& t : mTumblers)
            guess.push_back(t.GetDigit());

        if (guess == mFairyChest->mAnswer)
        {
            Unlocked();
        }

    }

    void Unlocked()
    {
        mUnlocked = true;

        mDialogScene.SetDialogFinished(
            [this](const auto&)
            {
                mGuiManager.ExitLock();
                mDialogScene.ResetDialogFinished();
            });

        mGuiManager.StartDialog(
            BAK::DialogSources::mOpenedWordlock,
            false,
            false,
            &mDialogScene);
    }

    void CantOpenLock()
    {
        mUnlocked = false;
        mDialogScene.SetDialogFinished(
            [this](const auto&)
            {
                mGuiManager.ExitLock();
                mDialogScene.ResetDialogFinished();
            });

        mGuiManager.StartDialog(
            BAK::DialogSources::mCantOpenWorldock,
            false,
            false,
            &mDialogScene);
    }

    void AddChildren()
    {
        AddChildBack(&mFrame);
        AddChildBack(&mExit);
        AddChildBack(&mDescription);

        for (auto& t : mTumblers)
            AddChildBack(&t);
    }

private:

    IGuiManager& mGuiManager;
    const Font& mAlienFont;
    const Font& mPuzzleFont;
    const Icons& mIcons;
    BAK::GameState& mGameState;
    DynamicDialogScene mDialogScene;

    std::optional<BAK::FairyChest> mFairyChest;
    BAK::Layout mLayout;

    Widget mFrame;
    ClickButtonImage mExit;
    Widget mLeftClasp;
    Widget mRightClasp;
    TextBox mDescription;

    using ClickableTumbler = Clickable<
        Tumbler,
        LeftMousePress,
        std::function<void()>>;
    std::vector<ClickableTumbler> mTumblers;

    std::vector<TextBox> mReqLocs;
    
    BAK::IContainer* mContainer;
    bool mNeedRefresh;

    bool mUnlocked;

    const Logging::Logger& mLogger;
};

}
