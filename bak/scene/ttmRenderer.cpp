#include "bak/scene/ttmRenderer.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/imageStore.hpp"
#include "bak/scene/scene.hpp"
#include "bak/scene/sceneData.hpp"
#include "bak/screen.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

namespace BAK {

TTMRenderer::TTMRenderer(
    std::string adsFile,
    std::string ttmFile)
:
    mRunner{},
    mLogger{Logging::LogState::GetLogger("TTMRenderer")}
{
    mRunner.LoadTTM(adsFile, ttmFile);
}

Graphics::TextureStore TTMRenderer::RenderTTM()
{
    while (AdvanceFrame()) {}
    return mRenderedFrames;
}

bool TTMRenderer::AdvanceFrame()
{
    auto frameOpt = mRunner.GetNextFrame();
    if (!frameOpt)
    {
        return false;
    }

    for (const auto& action : frameOpt->mActions)
    {
        mLogger.Debug() << "Handle action: " << action << std::endl;
        std::visit(
            overloaded{
                [&](const SlotPalette& sp){
                    mCurrentPaletteSlot = sp.mSlot;
                },
                [&](const LoadPalette& p){
                    mPaletteSlots.erase(mCurrentPaletteSlot);
                    mPaletteSlots.emplace(mCurrentPaletteSlot, Palette{p.mPalette});
                },
                [&](const SlotImage& sp){
                    mCurrentImageSlot = sp.mSlot;
                },
                [&](const LoadImage& p){
                    auto fb = FileBufferFactory::Get().CreateDataBuffer(p.mImage);
                    mImageSlots.erase(mCurrentImageSlot);
                    mImageSlots.emplace(mCurrentImageSlot, LoadImages(fb));
                    mLogger.Debug() << "Loaded image: " << p.mImage << " to slot: " << mCurrentImageSlot
                        << " has " << mImageSlots.at(mCurrentImageSlot).mImages.size() << " images\n";
                },
                [&](const LoadScreen& p){
                    if (!mPaletteSlots.contains(mCurrentPaletteSlot))
                    {
                        mLogger.Debug() << "No palette in slot " << mCurrentPaletteSlot
                            << ", skipping screen " << p.mScreenName << "\n";
                        return;
                    }
                    auto fb = FileBufferFactory::Get().CreateDataBuffer(p.mScreenName);
                    mRenderer.CopyImage(
                        LoadScreenResource(fb),
                        mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                        glm::ivec2{0, 0},
                        mRenderer.GetLayer(Layer::Screen));
                    mRenderer.CopyRect(
                        glm::ivec2{0, 0},
                        glm::ivec2{320, 200},
                        Layer::Screen,
                        Layer::Background);
                },
                [&](const CopyLayer& sa){
                    mRenderer.CopyRect(
                        sa.mPosition,
                        sa.mDimensions,
                        LayerFromArgument(sa.mSourceLayer),
                        LayerFromArgument(sa.mTargetLayer));
                },
                [&](const DrawSprite& sa){
                    assert(mImageSlots.contains(sa.mImageSlot));
                    assert(static_cast<unsigned>(sa.mSpriteIndex)
                            < mImageSlots.at(sa.mImageSlot).mImages.size());

                    mRenderer.RenderSprite(
                        mImageSlots.at(sa.mImageSlot).mImages[sa.mSpriteIndex],
                        mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                        glm::ivec2{sa.mX, sa.mY},
                        sa.mFlipX,
                        sa.mFlipY,
                        mRenderer.GetLayer(Layer::Screen));
                },
                [&](const SaveRectToBackground& si){
                    mRenderer.CopyRect(si.pos, si.dims, Layer::Screen, Layer::Background);
                },
                [&](const SetSaveLayer& ssl){
                    mImageSaveLayer = ssl.mLayer;
                },
                [&](const SaveRegionToLayer& si){
                    mClearRegions.insert_or_assign(mImageSaveLayer, si);
                    mSaves.insert_or_assign(
                        mImageSaveLayer,
                        mRenderer.ExtractRegion(si.pos, si.dims, Layer::Screen));
                },
                [&](const DrawSavedRegion& si){
                    mRenderer.CopyRect(
                        mSaves.at(si.mLayer),
                        mClearRegions.at(si.mLayer).pos,
                        mRenderer.GetLayer(Layer::Screen));
                },
                [&](const SaveBackground&){
                    mRenderer.CopyRect(
                        glm::ivec2{0, 0},
                        glm::ivec2{320, 200},
                        Layer::Screen,
                        Layer::Background);
                },
                [&](const DrawRect& sr){
                    if (!mPaletteSlots.contains(mCurrentPaletteSlot))
                    {
                        // what to do in this scenario..?
                        return;
                    }
                    mRenderer.DrawRect(
                        sr.mPos, sr.mDims,
                        mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                        sr.mFilled,
                        mRenderer.GetLayer(Layer::Screen));
                },
                [&](const ClipRegion& a){
                    mRenderer.SetClipRegion(a);
                },
                [&](const DisableClipRegion&){
                    mRenderer.ClearClipRegion();
                },
                [&](const SetColors& sc){
                    mRenderer.SetColors(sc.mForegroundColor, sc.mBackgroundColor);
                },
                [&](const EndScript&){},
                [&](const GotoTag&){},
                [&](const auto& a){
                    Logging::LogInfo(__FUNCTION__) << "Unhandled action: " << a << "\n";
                }
            },
            action
        );
    }

    RenderFrame();

    return true;
}
void TTMRenderer::RenderFrame()
{
    auto frame = mRenderer.GetLayer(Layer::Screen);
    frame.Invert();
    mRenderedFrames.AddTexture(frame);

    mRenderer.CopyRect(
        glm::ivec2{0, 0},
        glm::ivec2{320, 200},
        Layer::Background,
        Layer::Screen);
}
}
