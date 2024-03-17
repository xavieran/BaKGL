#include "bak/ttmRenderer.hpp"

#include "bak/dialogSources.hpp"
#include "bak/imageStore.hpp"
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
    mRunner{adsFile, ttmFile},
    mLogger{Logging::LogState::GetLogger("BAK::TTMRenderer")}
{
}

Graphics::TextureStore TTMRenderer::RenderTTM()
{
    while (!AdvanceAction())
    {
    }
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
            },
            [&](const BAK::SaveImage& si){
                mRenderer.SaveImage(si.pos, si.dims, mImageSaveLayer);
            },
            [&](const BAK::SetClearRegion& si){
                mClearRegions.emplace(mImageSaveLayer, si);
            },
            [&](const BAK::ClearSaveLayer& si){
                const auto& clearRegion = mClearRegions.at(mImageSaveLayer);
                mRenderer.ClearSaveLayer(clearRegion.pos, clearRegion.dims, si.mLayer);
            },
            [&](const BAK::SaveBackground&){
                mRenderer.GetSavedImagesLayer0() = {320, 200};
                mRenderer.GetSavedImagesLayer1() = {320, 200};
                mRenderer.SaveImage({0, 0}, {320, 200}, 2);
            },
            [&](const BAK::DrawRect& sr){
            },
            [&](const BAK::ClipRegion& a){
                mRenderer.SetClipRegion(a);
            },
            [&](const BAK::DisableClipRegion&){
                mRenderer.ClearClipRegion();
            },
            [&](const BAK::Purge&){
                assert(false);
            },
            [&](const BAK::GotoTag& sa){
                assert(false);
            },
            [&](const auto&){}
        },
        action
    );

    return false;
}

}
