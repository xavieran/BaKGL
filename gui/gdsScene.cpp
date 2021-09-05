#include "gui/gdsScene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

GDSScene::GDSScene(
    Cursor& cursor,
    BAK::HotspotRef hotspotRef,
    Graphics::SpriteManager& spriteManager,
    DialogRunner& dialogRunner)
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
    mMousePos{glm::vec2{0}},
    mHotspots{},
    mCursor{cursor},
    mDialogRunner{dialogRunner},
    mDialogState{false, false, glm::vec2{0}},
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

    SetDisplayScene();
}

void GDSScene::SetDisplayScene()
{
    mFrame.ClearChildren();
    assert(mStaticTTMs.size() > 0);
    mFrame.AddChildBack(mStaticTTMs.back().GetScene());
}

void GDSScene::SetDisplayBackground()
{
    mFrame.ClearChildren();
    assert(mStaticTTMs.size() > 0);
    mFrame.AddChildBack(mStaticTTMs.back().GetBackground());
}

void GDSScene::LeftMousePress(glm::vec2 pos)
{
    if (mDialogState.mDialogActive)
    {
        const auto dialogActive = mDialogState.RunDialog([&]{
            return mDialogRunner.RunDialog();
        });

        if (!dialogActive)
        {
            mLogger.Debug() << "Dialog finished, back to flavour text\n";
            mDialogRunner.ShowFlavourText(mFlavourText);

            if (mStaticTTMs.size() > 1)
                mStaticTTMs.pop_back();
            SetDisplayScene();

            mCursor.PopCursor();
        }
    }
    // Only propagate event if no dialog active
    else
    {
        Widget::LeftMousePress(pos);
    }
}

void GDSScene::HandleHotspotLeftClicked(const BAK::Hotspot& hotspot)
{ 
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";

    if (hotspot.mAction == BAK::HotspotAction::DIALOG
        && !mDialogState.mDialogActive) // arguably this shouldn't be necessary
    {
        if (hotspot.mActionArg2 != 0x0)
        {
            const auto gs = BAK::GameState{};
            const auto& scene1 = mSceneHotspots.GetScene(
                mSceneHotspots.mSceneIndex1, gs);
            const auto& scene2 = mSceneHotspots.GetScene(
                hotspot.mActionArg2, gs);

            // respect the reserve earlier
            assert(mStaticTTMs.size () < mMaxSceneNesting);
            mStaticTTMs.emplace_back(
                mSpriteManager,
                scene1,
                scene2);
            SetDisplayScene();
        }

        mDialogState.ActivateDialog();
        mDialogRunner.BeginDialog(
            BAK::KeyTarget{hotspot.mActionArg3});
        mCursor.PushCursor(0);
    }
}

void GDSScene::HandleHotspotRightClicked(const BAK::Hotspot& hotspot)
{
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";
    if (!mDialogState.mDialogActive)
    {
        mDialogRunner.BeginDialog(hotspot.mTooltip);
        mDialogState.ActivateTooltip(mMousePos);
    }
}

void GDSScene::MouseMoved(glm::vec2 pos)
{
    mMousePos = pos;

    mDialogState.DeactivateTooltip(
        pos,
        [&]{
            mDialogRunner.ShowFlavourText(mFlavourText);
        }
    );
    
    if (!mDialogState.mDialogActive)
        Widget::MouseMoved(pos);
}

}
