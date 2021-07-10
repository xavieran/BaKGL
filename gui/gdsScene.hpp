#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"

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

    GDSScene(BAK::HotspotRef hotspotRef)
    :
        mReference{hotspotRef},
        mHotspots{},
        mDrawActions{},
        mSprites{},
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
                auto drawAction = std::visit(
                    overloaded{
                        [&](const BAK::DrawSprite& sa) -> DrawingAction {
                            return ConvertSceneAction(
                                sa,
                                textures,
                                offsets);
                        },
                        [&](const BAK::DrawRect& sr) -> DrawingAction {
                            const auto [palKey, colorKey] = sr.mPaletteColor;
                            assert(textures.size() > 0);
                            const auto [iwidth, iheight] = textures.GetTexture(0).GetDims();
                            const auto width  = static_cast<float>(iwidth);
                            const auto height = static_cast<float>(iheight);
                            const auto scale = glm::vec3{
                                sr.mBottomRight.x / width,
                                sr.mBottomRight.y / height,
                                0};
                            return SceneRect{
                                colorKey == 6 
                                    ? glm::vec4{0.3, 0.11, 0.094, 1.0}
                                    : glm::vec4{1.0, 1.0, 1.0, 1.0},
                                glm::vec3{sr.mTopLeft.x, sr.mTopLeft.y, 0},
                                glm::vec3{scale}
                            };
                        },
                        [](auto&& a) -> DrawingAction {
                            return ConvertSceneAction(a);
                        }
                    },
                    action);

                if (std::holds_alternative<SceneRect>(drawAction))
                    mDrawActions.insert(mDrawActions.begin(), drawAction);
                else
                    mDrawActions.emplace_back(drawAction);
            }
        }

        mDrawActions.insert(
            mDrawActions.begin(),
            SceneSprite{
                0,
                glm::vec3{0},
                glm::vec3{1}});

        mSprites.LoadTexturesGL(textures);
    }

    GDSScene& operator=(GDSScene&& other)
    {
        this->mHotspots = std::move(other.mHotspots);
        this->mDrawActions = std::move(other.mDrawActions);
        this->mSprites = std::move(other.mSprites);
        return *this;
    }

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

    BAK::HotspotRef mReference;
    BAK::SceneHotspots mHotspots;
    std::vector<DrawingAction> mDrawActions;
    Graphics::Sprites mSprites;

    const Logging::Logger& mLogger;
};

}
