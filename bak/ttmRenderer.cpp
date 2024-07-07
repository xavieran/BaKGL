#include "bak/ttmRenderer.hpp"

#include "bak/dialogSources.hpp"
#include "bak/imageStore.hpp"
#include "bak/sceneData.hpp"
#include "bak/screen.hpp"
#include "bak/textureFactory.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/types.hpp"

namespace BAK {

TTMRenderer::TTMRenderer(
    std::string adsFile,
    std::string ttmFile)
:
    mRunner{},
    mLogger{Logging::LogState::GetLogger("BAK::TTMRenderer")}
{
    mRunner.LoadTTM(adsFile, ttmFile);
}

Graphics::TextureStore TTMRenderer::RenderTTM()
{
    while (!AdvanceAction()) {}
    return mRenderedFrames;
}

bool TTMRenderer::AdvanceAction()
{
    mLogger.Debug() << "AdvanceAction" << "\n";
    auto actionOpt = mRunner.GetNextAction();
    if (!actionOpt)
    {
        return true;
    }
    auto action = *actionOpt;
    mLogger.Debug() << "Handle action: " << action << std::endl;
    std::visit(
        overloaded{
            [&](const BAK::SlotPalette& sp){
                mCurrentPaletteSlot = sp.mSlot;
            },
            [&](const BAK::LoadPalette& p){
                mPaletteSlots.erase(mCurrentPaletteSlot);
                mPaletteSlots.emplace(mCurrentPaletteSlot, BAK::Palette{p.mPalette});
            },
            [&](const BAK::SlotImage& sp){
                mCurrentImageSlot = sp.mSlot;
            },
            [&](const BAK::LoadImage& p){
                auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(p.mImage);
                mImageSlots.erase(mCurrentImageSlot);
                mImageSlots.emplace(mCurrentImageSlot, BAK::LoadImages(fb));
                mLogger.Debug() << "Loaded image: " << p.mImage << " to slot: " << mCurrentImageSlot
                    << " has " << mImageSlots.at(mCurrentImageSlot).mImages.size() << " images\n";
            },
            [&](const BAK::LoadScreen& p){
                auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(p.mScreenName);
                mScreen = BAK::LoadScreenResource(fb);
            },
            [&](const BAK::DrawScreen& sa){
                if (sa.mArg1 == 3 || sa.mArg2 == 3)
                {
                    mRenderer.GetSavedImagesLayer0() = {320, 200};
                    mRenderer.GetSavedImagesLayer1() = {320, 200};
                    mRenderer.GetSavedImagesLayerBG() = {320, 200};
                }
                if (mScreen && mPaletteSlots.contains(mCurrentPaletteSlot))
                {
                    mRenderer.RenderSprite(
                        *mScreen,
                        mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                        glm::ivec2{0, 0},
                        false,
                        mRenderer.GetForegroundLayer());
                }
            },
            [&](const BAK::DrawSprite& sa){
                const auto imageSlot = sa.mImageSlot;
                assert(mImageSlots.contains(sa.mImageSlot));
                assert(static_cast<unsigned>(sa.mSpriteIndex) 
                        < mImageSlots.at(sa.mImageSlot).mImages.size());

                mRenderer.RenderSprite(
                    mImageSlots.at(sa.mImageSlot).mImages[sa.mSpriteIndex],
                    mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                    glm::ivec2{sa.mX, sa.mY},
                    sa.mFlippedInY,
                    mRenderer.GetForegroundLayer());
            },
            [&](const BAK::Update& sr){
                RenderFrame();
            },
            [&](const BAK::SaveImage& si){
                mRenderer.SaveImage(si.pos, si.dims, mImageSaveLayer);
            },
            [&](const BAK::SetSaveLayer& ssl){
                mImageSaveLayer = ssl.mLayer;
            },
            [&](const BAK::SaveRegionToLayer& si){
                mClearRegions.emplace(mImageSaveLayer, si);
                mSaves.emplace(mImageSaveLayer, mRenderer.SaveImage(si.pos, si.dims, mImageSaveLayer));
            },
            [&](const BAK::DrawSavedRegion& si){
                const auto& clearRegion = mClearRegions.at(si.mLayer);
                const auto& texture = mSaves.at(si.mLayer);
                mRenderer.RenderTexture(texture, clearRegion.pos, mRenderer.GetForegroundLayer());
            },
            [&](const BAK::SaveBackground&){
                mRenderer.GetSavedImagesLayer0() = {320, 200};
                mRenderer.GetSavedImagesLayer1() = {320, 200};
                mRenderer.SaveImage({0, 0}, {320, 200}, 2);
            },
            [&](const BAK::DrawRect& sr){
                if (!mPaletteSlots.contains(mCurrentPaletteSlot))
                {
                    // what to do in this scenario..?
                    return;
                }
                mRenderer.DrawRect(
                    sr.mPos, sr.mDims,
                    mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                    mRenderer.GetForegroundLayer());
            },
            [&](const BAK::ClipRegion& a){
                mRenderer.SetClipRegion(a);
            },
            [&](const BAK::DisableClipRegion&){
                mRenderer.ClearClipRegion();
            },
            [&](const BAK::SetColors& sc){
                mRenderer.SetColors(sc.mForegroundColor, sc.mBackgroundColor);
            },
            [&](const BAK::Purge&){
                assert(false);
            },
            [&](const BAK::GotoTag& sa){
                assert(false);
            },
            [&](const auto& a){
                Logging::LogInfo(__FUNCTION__) << "Unhandled action: " << a << "\n";
            }
        },
        action
    );

    return false;
}
void TTMRenderer::RenderFrame()
{
    if (mScreen)
    {
        mRenderer.RenderSprite(
            *mScreen,
            mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
            glm::ivec2{0, 0}, false, mRenderer.GetBackgroundLayer());
    }

    mRenderer.RenderTexture(
        mRenderer.GetSavedImagesLayerBG(),
        glm::ivec2{0},
        mRenderer.GetBackgroundLayer());

    mRenderer.RenderTexture(
        mRenderer.GetSavedImagesLayer0(),
        glm::ivec2{0},
        mRenderer.GetBackgroundLayer());

    mRenderer.RenderTexture(
        mRenderer.GetSavedImagesLayer1(),
        glm::ivec2{0},
        mRenderer.GetBackgroundLayer());

    mRenderer.RenderTexture(
        mRenderer.GetForegroundLayer(),
        glm::ivec2{0, 0},
        mRenderer.GetBackgroundLayer());

    auto bg = mRenderer.GetBackgroundLayer();
    bg.Invert();
    mRenderedFrames.AddTexture(bg);

    mRenderer.GetForegroundLayer() = Graphics::Texture{320, 200};
    mRenderer.GetBackgroundLayer() = Graphics::Texture{320, 200};
}
}
