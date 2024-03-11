#include "gui/dynamicTTM.hpp"

#include "bak/imageStore.hpp"
#include "bak/screen.hpp"
#include "bak/textureFactory.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/types.hpp"

#include "gui/colors.hpp"

namespace Gui {

DynamicTTM::DynamicTTM(
    Graphics::SpriteManager& spriteManager,
    std::string adsFile,
    std::string ttmFile)
:
    mSpriteManager{spriteManager},
    mSceneFrame{
        Graphics::DrawMode::Rect,
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        glm::vec4{0},
        glm::vec2{0},
        glm::vec2{1},
        false 
    },
    mDialogBackground{},
    mSceneElements{},
    mLogger{Logging::LogState::GetLogger("Gui::DynamicTTM")}
{
    mSceneElements.reserve(100);
    mLogger.Debug() << "Loading ADS/TTM: " << adsFile << " " << ttmFile << "\n";
    auto adsFb = BAK::FileBufferFactory::Get().CreateDataBuffer(adsFile);
    mSceneSequences = BAK::LoadSceneSequences(adsFb);
    auto ttmFb = BAK::FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    mScenes = BAK::LoadDynamicScenes(ttmFb);
}

void DynamicTTM::BeginScene()
{

    mLogger.Debug() << "SceneSequences\n";
    for (const auto& [key, sequences] : mSceneSequences)
    {
        mLogger.Debug() << "Key: " << key << "\n";
        for (const auto& sequence : sequences)
        {
            mLogger.Debug() << "  Sequence: " << sequence.mName << "\n";
            for (const auto& scene : sequence.mScenes)
            {
                mLogger.Debug() << "    ADS(" << scene.mInitScene << ", " << scene.mDrawScene << ")\n";
            }
        }
    }
    mLogger.Debug() << "Scenes" << "\n";
    for (const auto& [key, scene] : mScenes)
    {
        mLogger.Debug() << "Key: " << key << " - " << scene.mSceneTag << "\n";
    }

    if (mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mPlayAllScenes)
    {
        for (const auto& [key, _] : mScenes)
        {
            mAllSceneKeys.emplace_back(key);
        }
        mPlayAllScenes = true;
        mCurrentScene = &(mScenes[mAllSceneKeys[0]]);
    }
    else
    {
        mCurrentScene = &(mScenes[mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene]);
    }
    mLogger.Debug() << "Starting at: " << mCurrentScene->mSceneTag << "\n";
    mCurrentAction = 0;
}

void DynamicTTM::AdvanceAction()
{
    mLogger.Debug() << "AdvanceAction" << "\n";
    const auto& action = mCurrentScene->mActions[mCurrentAction++];
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
                mRenderer.GetSavedZonesLayer() = {320, 200};
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
                auto textures = Graphics::TextureStore{};
                if (mScreen)
                {
                    mRenderer.RenderSprite(*mScreen, mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData, glm::ivec2{0, 0}, false, mRenderer.GetBackgroundLayer());
                }
                if (mBackgroundImage)
                {
                    const auto& texture = *mBackgroundImage;
                    mRenderer.RenderTexture(texture, glm::ivec2{0, 0}, mRenderer.GetBackgroundLayer());
                }
                mRenderer.RenderTexture(mRenderer.GetSavedZonesLayer(), glm::ivec2{0}, mRenderer.GetBackgroundLayer());
                mRenderer.RenderTexture(
                    mRenderer.GetForegroundLayer(),
                    glm::ivec2{0, 0},
                    mRenderer.GetBackgroundLayer());

                auto bg = mRenderer.GetBackgroundLayer();
                bg.Invert();
                textures.AddTexture(bg);

                //auto fg = mRenderer.GetForegroundLayer();
                //fg.Invert();
                //textures.AddTexture(fg);

                auto temporarySpriteSheet = mSpriteManager.AddTemporarySpriteSheet();
                mImageSprites[0] = std::move(temporarySpriteSheet);
                mSpriteManager.GetSpriteSheet(mImageSprites[0]->mSpriteSheet).LoadTexturesGL(textures);

                mSceneElements.clear();
                mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mImageSprites[0]->mSpriteSheet,
                    Graphics::TextureIndex{0},
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec2{0},
                    glm::vec2{320, 200},
                    false 
                );
                //mSceneElements.emplace_back(
                //    Graphics::DrawMode::Sprite,
                //    mImageSprites[0]->mSpriteSheet,
                //    Graphics::TextureIndex{1},
                //    Graphics::ColorMode::Texture,
                //    glm::vec4{1},
                //    glm::vec2{0},
                //    glm::vec2{320, 200},
                //    false 
                //);
                mSceneFrame.ClearChildren();
                for (auto& element : mSceneElements)
                {
                    mSceneFrame.AddChildBack(&element);
                }
                mRenderer.GetForegroundLayer() = Graphics::Texture{320, 200};
                mRenderer.GetBackgroundLayer() = Graphics::Texture{320, 200};
            },
            [&](const BAK::SaveImage& si){
                mRenderer.SaveImage(si.pos, si.dims);
            },
            [&](const BAK::SaveBackground&){
                mRenderer.SaveImage({0, 0}, {320, 200});
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
                mRenderer.ClearClipRegion();
                mRenderer.Clear();
            },
            [&](const auto&){}
        },
        action
    );

    if (mCurrentAction == mCurrentScene->mActions.size())
    {
        mBackgroundImage.reset();
        mLogger.Debug() << "Reached end of scene, next scene is: \n";
        mRenderer.Clear(); // is this always the case..?
        if (mPlayAllScenes)
        {
            mCurrentSequenceScene++;
            auto it = mAllSceneKeys.begin();
            std::advance(it, mCurrentSequenceScene);
            if (it != mAllSceneKeys.end())
            {
                mLogger.Debug() << "Next scene key is: " << *it << " from sequence: "
                    << mCurrentSequenceScene << " -- " << mScenes[*it].mSceneTag << "\n";
                mCurrentScene = &(mScenes[*it]);
            }
            else
            {
                mCurrentSequenceScene = 0;
                mCurrentScene = &(mScenes[mAllSceneKeys[0]]);
            }

            mCurrentAction = 0;
            return;
        }
        mCurrentSequenceScene++;
        if (mCurrentSequenceScene == mSceneSequences[1][mCurrentSequence].mScenes.size())
        {
            mCurrentSequenceScene = 0;
            mCurrentSequence++;
        }

        if (mCurrentSequence == mSceneSequences[1].size())
        {
            mLogger.Debug() << "Finished, start again\n";
            mCurrentSequence = 0;
        }
        const auto scene = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
        mCurrentScene = &mScenes[scene];
        mLogger.Debug() << "Sequence: " << mCurrentSequence << " Scene: " << mCurrentSequenceScene << 
            " -- " << scene << " " << mCurrentScene->mSceneTag << "\n";
        mCurrentAction = 0;
    }
}

Widget* DynamicTTM::GetScene()
{
    return &mSceneFrame;
}

}
