#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/colors.hpp"
#include "gui/fixedGuiElement.hpp"
#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GDSScene : public FixedGuiElement
{
public:

    GDSScene(
        BAK::HotspotRef hotspotRef,
        Graphics::SpriteManager& spriteManager)
    :
        FixedGuiElement{
            Graphics::DrawMode::Sprite,
            std::invoke([&spriteManager]{
                const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
                return sheetIndex;
            }),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec3{0},
            glm::vec3{1},
            false
        },
        mReference{hotspotRef},
        mHotspots{},
        mSpriteSheet{GetDrawInfo().mSpriteSheet},
        mFrame{
            Graphics::DrawMode::Rect,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec3{0},
            glm::vec3{1},
           false 
        },
        mClipRegion{
            Graphics::DrawMode::ClipRegion,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec3{0},
            glm::vec3{1},
            false
        },
        mSceneElements{},
        mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
    {

        auto fb = FileBufferFactory::CreateFileBuffer(mReference.ToFilename());
        mHotspots.Load(fb);

        mLogger.Debug() << "ADS: " << mHotspots.mSceneADS
            << " TTM: " << mHotspots.mSceneTTM
            << " " << mHotspots.mSceneIndex1
            << " " << mHotspots.mSceneIndex2 << "\n";

        auto fb2 = FileBufferFactory::CreateFileBuffer(mHotspots.mSceneADS);
        auto sceneIndices = BAK::LoadSceneIndices(fb2);
        auto fb3 = FileBufferFactory::CreateFileBuffer(mHotspots.mSceneTTM);
        auto scenes = BAK::LoadScenes(fb3);

        const auto& scene1 = scenes[sceneIndices[mHotspots.mSceneIndex1].mSceneIndex];
        const auto& scene2 = scenes[sceneIndices[mHotspots.mSceneIndex2].mSceneIndex];

        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "DIALOG.SCX", "OPTIONS.PAL");

        std::unordered_map<unsigned, unsigned> offsets{};

        // We do some pretty wacky stuff here to end up with 
        // Background
        // -> Rect
        // -> ClipRegion
        // ---> Scene Elements

        for (const auto& scene : {scene1, scene2})
        {
            for (const auto& [imageKey, imagePal] : scene.mImages)
            {
                const auto& [image, palKey] = imagePal;
                const auto& palette = scene.mPalettes.find(palKey)->second;
                offsets[imageKey] = textures.GetTextures().size();
                BAK::TextureFactory::AddToTextureStore(
                    textures,
                    image,
                    palette);
            }

            for (const auto& action : scene.mActions)
            {
                std::visit(
                    overloaded{
                        [&](const BAK::DrawSprite& sa){
                            const auto sceneSprite = ConvertSceneAction(
                                sa,
                                textures,
                                offsets);

                            mSceneElements.emplace_back(
                                Graphics::DrawMode::Sprite,
                                mSpriteSheet,
                                sceneSprite.mImage,
                                Graphics::ColorMode::Texture,
                                glm::vec4{1},
                                sceneSprite.mPosition,
                                sceneSprite.mScale,
                                false);
                        },
                        [&](const BAK::DrawRect& sr){
                            const auto [palKey, colorKey] = sr.mPaletteColor;
                            const auto sceneRect = SceneRect{
                                // Reddy brown frame color
                                colorKey == 6 
                                    ? Gui::Color::frameMaroon
                                    : Gui::Color::black,
                                glm::vec3{sr.mTopLeft.x, sr.mTopLeft.y, 0},
                                glm::vec3{sr.mBottomRight.x, sr.mBottomRight.y, 0}};

                            mFrame = FixedGuiElement{
                                Graphics::DrawMode::Rect,
                                mSpriteSheet,
                                0, // no image index
                                Graphics::ColorMode::SolidColor,
                                sceneRect.mColor,
                                sceneRect.mPosition,
                                sceneRect.mDimensions,
                                false};

                            this->AddChildFront(&mFrame);
                        },
                        [&](const BAK::ClipRegion& a){
                            const auto clip = ConvertSceneAction(a);
                            mClipRegion = FixedGuiElement{
                                Graphics::DrawMode::ClipRegion,
                                mSpriteSheet,
                                0, // no image index
                                Graphics::ColorMode::SolidColor,
                                glm::vec4{1},
                                glm::vec3{clip.mTopLeft.x, clip.mTopLeft.y, 0},
                                glm::vec3{clip.mDims.x, clip.mDims.y, 0},
                                false};
                            this->AddChildBack(&mClipRegion);
                        },
                        [&](const BAK::DisableClipRegion&){

                        }
                    },
                    action);
            }
        }

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
        Graphics::IGuiElement* addTo = this->mChildren.size() > 1
            ? this->mChildren.back()
            : this;

        for (auto& action : mSceneElements)
        {
            addTo->AddChildBack(&action);
        }
    }

    GDSScene& operator=(GDSScene&& other)
    {
        this->mHotspots = std::move(other.mHotspots);
        return *this;
    }

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

    BAK::HotspotRef mReference;
    BAK::SceneHotspots mHotspots;

    Graphics::SpriteSheetIndex mSpriteSheet;
    FixedGuiElement mFrame;
    FixedGuiElement mClipRegion;
    std::vector<FixedGuiElement> mSceneElements;

    const Logging::Logger& mLogger;
};

}
