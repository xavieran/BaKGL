#include "gui/lock/moredhelScreen.hpp"

#include "bak/lock.hpp"
#include "bak/IContainer.hpp"
#include "bak/dialogSources.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/backgrounds.hpp"
#include "gui/core/clickable.hpp"
#include "gui/icons.hpp"
#include "gui/colors.hpp"
#include "gui/clickButton.hpp"
#include "gui/textBox.hpp"
#include "gui/core/widget.hpp"

#include <glm/glm.hpp>

#include <iostream>

namespace Gui {


MoredhelScreen::MoredhelScreen(
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
    mUnlocked{false}
{
}

void MoredhelScreen::ResetUnlocked()
{
    mUnlocked = false;
}

bool MoredhelScreen::IsUnlocked() const
{
    return mUnlocked;
}

BAK::IContainer* MoredhelScreen::GetContainer() const
{
    return mContainer;
}

void MoredhelScreen::SetContainer(BAK::IContainer* container)
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

void MoredhelScreen::RefreshGui()
{
    ClearChildren();

    UpdateTumblers();

    mDescription.SetText(
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
        mReqLocs.back().SetText(mPuzzleFont, ss.str());
    }

    AddChildren();
}

void MoredhelScreen::UpdateTumblers()
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

void MoredhelScreen::IncrementTumbler(unsigned tumblerIndex)
{
    ASSERT(tumblerIndex < mTumblers.size());
    mTumblers[tumblerIndex].NextDigit();

    EvaluateLock();
}

void MoredhelScreen::EvaluateLock()
{
    std::string guess{};
    for (const auto& t : mTumblers)
        guess.push_back(t.GetDigit());

    if (guess == mFairyChest->mAnswer)
    {
        Unlocked();
    }

}

void MoredhelScreen::Unlocked()
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

void MoredhelScreen::CantOpenLock()
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

void MoredhelScreen::AddChildren()
{
    AddChildBack(&mFrame);
    AddChildBack(&mExit);
    AddChildBack(&mDescription);

    for (auto& t : mTumblers)
        AddChildBack(&t);
}

}
