#include "gui/gdsScene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

GDSScene::GDSScene(
    Cursor& cursor,
    BAK::HotspotRef hotspotRef,
    Graphics::SpriteManager& spriteManager,
    const Actors& actors,
    const Backgrounds& backgrounds,
    const Font& font,
    BAK::GameState& gameState,
    ScreenStack& screenStack,
    IGuiManager& guiManager)
:
    Widget{
        Graphics::DrawMode::Sprite,
        spriteManager.AddSpriteSheet(),
        Graphics::TextureIndex{0},
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        glm::vec2{0},
        glm::vec2{1},
        false
    },
    mReference{hotspotRef},
    mSceneHotspots{
        FileBufferFactory::CreateFileBuffer(
            mReference.ToFilename())},
    mFlavourText{BAK::KeyTarget{0x10000}},
    mSpriteSheet{GetDrawInfo().mSpriteSheet},
    mSpriteManager{spriteManager},
    // bitofa hack - all gds scenes have such a frame
    mFrame{
        Graphics::DrawMode::Rect,
        mSpriteSheet,
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        Color::frameMaroon,
        glm::vec2{14, 10},
        glm::vec2{291, 103},
        false},
    mStaticTTMs{},
    mHotspots{},
    mCursor{cursor},
    mScreenStack{screenStack},
    mGuiManager{guiManager},
    mDialogDisplay{
        glm::vec2{0, 0},
        glm::vec2{320, 240},
        actors,
        backgrounds,
        font,
        gameState},
    mDialogRunner{
        glm::vec2{0, 0},
        glm::vec2{320, 240},
        actors,
        backgrounds,
        font,
        gameState,
        screenStack,
        *this, // IDialogScene
        [this](){ FinishedDialog(); }},
    mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
{
    auto textures = Graphics::TextureStore{};
    BAK::TextureFactory::AddScreenToTextureStore(
        textures, "DIALOG.SCX", "OPTIONS.PAL");

    const auto [x, y] = textures.GetTexture(0).GetDims();
    mSpriteManager
        .GetSpriteSheet(mSpriteSheet)
        .LoadTexturesGL(textures);

    SetDimensions(glm::vec2{x, y});

    auto fb = FileBufferFactory::CreateFileBuffer(mReference.ToFilename());
    mFlavourText = BAK::KeyTarget{mSceneHotspots.mFlavourText};

    const auto gs = BAK::GameState{};
    const auto& scene1 = mSceneHotspots.GetScene(
        mSceneHotspots.mSceneIndex1, gs);
    const auto& scene2 = mSceneHotspots.GetScene(
        mSceneHotspots.mSceneIndex2, gs);

    // Unlikely we ever nest this deep
    mStaticTTMs.reserve(mMaxSceneNesting);
    mStaticTTMs.emplace_back(
        mSpriteManager,
        scene1,
        scene2);

    AddChildBack(&mFrame);
    mFrame.SetInactive();

    // Want our refs to be stable..
    mHotspots.reserve(mSceneHotspots.mHotspots.size());
    for (const auto& hs : mSceneHotspots.mHotspots)
    {
        mHotspots.emplace_back(
            cursor,
            hs.mTopLeft,
            hs.mDimensions,
            hs.mKeyword - 1, // cursor index
            [this, hs](){
                HandleHotspotLeftClicked(hs);
            },
            [this, hs](){
                HandleHotspotRightClicked(hs);
            });

        AddChildBack(&mHotspots.back());
    }

    DisplayNPCBackground();

    mDialogDisplay.ShowFlavourText(mFlavourText);
    AddChildBack(&mDialogDisplay);

    mScreenStack.PushScreen(this);
    mLogger.Debug() << "Constructed @" << std::hex << this << std::dec << "\n";
}

void GDSScene::DisplayNPCBackground()
{
    mFrame.ClearChildren();
    assert(mStaticTTMs.size() > 0);
    mFrame.AddChildBack(mStaticTTMs.back().GetScene());
}

void GDSScene::DisplayPlayerBackground()
{
    mFrame.ClearChildren();
    assert(mStaticTTMs.size() > 0);
    mFrame.AddChildBack(mStaticTTMs.back().GetBackground());
}

void GDSScene::HandleHotspotLeftClicked(const BAK::Hotspot& hotspot)
{ 
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";

    if (hotspot.mAction == BAK::HotspotAction::DIALOG
        && !mDialogRunner.Active())
    {
        if (hotspot.mActionArg2 != 0x0)
        {
            const auto gs = BAK::GameState{};
            const auto& scene1 = mSceneHotspots.GetScene(
                mSceneHotspots.mSceneIndex1, gs);
            const auto& scene2 = mSceneHotspots.GetScene(
                hotspot.mActionArg2, gs);

            // respect the earlier reserve
            assert(mStaticTTMs.size () < mMaxSceneNesting);
            mStaticTTMs.emplace_back(
                mSpriteManager,
                scene1,
                scene2);
            DisplayNPCBackground();
        }

        StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::EXIT)
    {
        mGuiManager.ExitGDSScene();
    }
    else if (hotspot.mAction == BAK::HotspotAction::GOTO)
    {
        auto hotspotRef = mReference;
        hotspotRef.mGdsChar = BAK::MakeHotspotChar(hotspot.mActionArg1);
        mGuiManager.EnterGDSScene(hotspotRef);
    }
}

void GDSScene::HandleHotspotRightClicked(const BAK::Hotspot& hotspot)
{
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";
    if (!mDialogRunner.Active())
    {
        StartDialog(hotspot.mTooltip, true);
    }
}

void GDSScene::StartDialog(const BAK::Target target, bool isTooltip)
{
    mDialogDisplay.Clear();
    mCursor.PushCursor(0);
    mDialogRunner.BeginDialog(target, isTooltip);
    mScreenStack.PushScreen(&mDialogRunner);
}

void GDSScene::FinishedDialog()
{
    mLogger.Debug() << "Dialog finished, back to flavour text\n";

    mScreenStack.PopScreen();
    mDialogDisplay.ShowFlavourText(mFlavourText);
    if (mStaticTTMs.size() > 1)
        mStaticTTMs.pop_back();
    DisplayNPCBackground();
    mCursor.PopCursor();
}

}
