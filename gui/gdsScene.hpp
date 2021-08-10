#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GDSScene
{
public:

    GDSScene(
        BAK::HotspotRef hotspotRef,
        Graphics::SpriteManager& spriteManager)
    :
        mReference{hotspotRef},
        mHotspots{},
        mSpriteSheet{std::invoke([&spriteManager]{
            const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
            return sheetIndex;
        })},
        mGuiElement{
            Graphics::DrawMode::Sprite,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec3{0},
            glm::vec3{1},
            false 
        },
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

        std::vector<Graphics::IGuiElement*> drawables{};

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

                            auto* action = new Graphics::IGuiElement{
                                Graphics::DrawMode::Sprite,
                                mSpriteSheet,
                                sceneSprite.mImage,
                                Graphics::ColorMode::Texture,
                                glm::vec4{1},
                                sceneSprite.mPosition,
                                sceneSprite.mScale,
                                false};
                            drawables.emplace_back(action);
                        },
                        [&](const BAK::DrawRect& sr){
                            const auto [palKey, colorKey] = sr.mPaletteColor;
                            const auto sceneRect = SceneRect{
                                // Reddy brown frame color
                                colorKey == 6 
                                    ? glm::vec4{0.3, 0.11, 0.094, 1.0}
                                    : glm::vec4{1.0, 1.0, 1.0, 1.0},
                                glm::vec3{sr.mTopLeft.x, sr.mTopLeft.y, 0},
                                glm::vec3{sr.mBottomRight.x, sr.mBottomRight.y, 0}};

                            auto* action = new Graphics::IGuiElement{
                                Graphics::DrawMode::Rect,
                                mSpriteSheet,
                                0, // no image index
                                Graphics::ColorMode::SolidColor,
                                sceneRect.mColor,
                                sceneRect.mPosition,
                                sceneRect.mDimensions,
                                false};
                            mGuiElement.AddChildFront(action);
                        },
                        [&](const BAK::ClipRegion& a){
                            const auto clip = ConvertSceneAction(a);
                            auto* action = new Graphics::IGuiElement{
                                Graphics::DrawMode::ClipRegion,
                                mSpriteSheet,
                                0, // no image index
                                Graphics::ColorMode::SolidColor,
                                glm::vec4{1},
                                glm::vec3{clip.mTopLeft.x, clip.mTopLeft.y, 0},
                                glm::vec3{clip.mDims.x, clip.mDims.y, 0},
                                true};

                            mGuiElement.AddChildBack(action);
                        },
                        [&](const BAK::DisableClipRegion&){

                        }
                    },
                    action);
            }
        }

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);
        Graphics::IGuiElement* addTo = mGuiElement.mChildren.size() > 1
            ? mGuiElement.mChildren.back()
            : &mGuiElement;

        for (const auto& action : drawables)
        {
            // do this to account for the position of the clip region...
            // children are rendered relative to their parent...
            action->mPosition -= addTo->mPosition;
            addTo->AddChildBack(action);
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
    Graphics::IGuiElement mGuiElement;

    const Logging::Logger& mLogger;
};

}
