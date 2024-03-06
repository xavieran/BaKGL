#include "gui/dynamicTTM.hpp"

#include "bak/imageStore.hpp"
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
    mClipRegion{},
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
            mLogger.Debug() << "  Sequence\n";
            for (const auto& scene : sequence.mScenes)
            {
                mLogger.Debug() << "    ADS(" << scene.mInitScene << ", " << scene.mDrawScene << ")\n";
            }
        }
    }
    mLogger.Debug() << "Scenes" << "\n";
    for (const auto& [key, scene] : mScenes)
    {
        mLogger.Debug() << "Key: " << key << "\n";
    }

    mCurrentScene = &(mScenes[mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene]);
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

                if (mImageSlots.contains(mCurrentImageSlot)
                    && mPaletteSlots.contains(mCurrentPaletteSlot))
                {
                    auto textures = Graphics::TextureStore{};
                    auto& palette = mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData;
                    BAK::TextureFactory::AddToTextureStore(
                        textures,
                        mImageSlots.at(mCurrentImageSlot).mImages,
                        palette);
                        
                    auto temporarySpriteSheet = mSpriteManager.AddTemporarySpriteSheet();
                    mImageSprites[mCurrentImageSlot] = std::move(temporarySpriteSheet);
                    mTextures[mCurrentImageSlot] = textures;
                    mSpriteManager.GetSpriteSheet(mImageSprites[mCurrentImageSlot]->mSpriteSheet).LoadTexturesGL(textures);
                }
            },
            [&](const BAK::LoadPalette& p){
                mPaletteSlots.emplace(mCurrentPaletteSlot, BAK::Palette{p.mPalette});
                if (mImageSlots.contains(mCurrentImageSlot)
                    && mPaletteSlots.contains(mCurrentPaletteSlot))
                {
                    auto textures = Graphics::TextureStore{};
                    auto& palette = mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData;
                    BAK::TextureFactory::AddToTextureStore(
                        textures,
                        mImageSlots.at(mCurrentImageSlot).mImages,
                        palette);
                        
                    auto temporarySpriteSheet = mSpriteManager.AddTemporarySpriteSheet();
                    mImageSprites[mCurrentImageSlot] = std::move(temporarySpriteSheet);
                    mTextures[mCurrentImageSlot] = textures;
                    mSpriteManager.GetSpriteSheet(mImageSprites[mCurrentImageSlot]->mSpriteSheet).LoadTexturesGL(textures);
                }
            },
            [&](const BAK::SlotImage& sp){
                mCurrentImageSlot = sp.mSlot;
            },
            [&](const BAK::LoadImage& p){
                auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(p.mImage);
                mImageSlots.erase(mCurrentImageSlot);
                mImageSlots.emplace(mCurrentImageSlot, BAK::LoadImages(fb));
                auto textures = Graphics::TextureStore{};
                auto& palette = mPaletteSlots.contains(mCurrentImageSlot)
                    ? mPaletteSlots.at(mCurrentImageSlot).mPaletteData
                    : mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData;
                BAK::TextureFactory::AddToTextureStore(
                    textures,
                    mImageSlots.at(mCurrentImageSlot).mImages,
                    palette);
                    
                auto temporarySpriteSheet = mSpriteManager.AddTemporarySpriteSheet();
                mImageSprites[mCurrentImageSlot] = std::move(temporarySpriteSheet);
                mTextures[mCurrentImageSlot] = textures;
                mSpriteManager.GetSpriteSheet(mImageSprites[mCurrentImageSlot]->mSpriteSheet).LoadTexturesGL(textures);
                mLogger.Debug() << "Loaded image: " << p.mImage << " has " << textures.size() << " textures to slot: " << mCurrentImageSlot << " and pal slot: " << mCurrentPaletteSlot << "\n";
            },
            [&](const BAK::DrawScreen& sa){
            },
            [&](const BAK::DrawSprite& sa){
                const auto imageSlot = sa.mImageSlot;
                const auto sceneSprite = ConvertSceneAction(
                    sa,
                    mTextures[imageSlot]);
                auto& elem = mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mImageSprites[imageSlot]->mSpriteSheet,
                    Graphics::TextureIndex{sceneSprite.mImage},
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    sceneSprite.mPosition,
                    sceneSprite.mScale,
                    false);
                mSceneFrame.AddChildBack(&elem);
            },
            [&](const BAK::Update& sr){
                mSceneFrame.ClearChildren();
            },
            [&](const BAK::DrawRect& sr){
            },
            [&](const BAK::ClipRegion& a){
            },
            [&](const BAK::DisableClipRegion&){
            },
            [&](const auto&){}
        },
        action
    );

    if (mCurrentAction == mCurrentScene->mActions.size())
    {
        mLogger.Debug() << "Reached end of scene, next scene is: \n";
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

Widget* DynamicTTM::GetBackground()
{
    ASSERT(mDialogBackground);
    return &(*mDialogBackground);
}

}
