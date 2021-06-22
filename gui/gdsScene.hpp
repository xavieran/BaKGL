#pragma once

#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"

#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace Gui {

class GDSScene
{
public:

    GDSScene(BAK::HotspotRef hotspotRef)
    :
        mHotspots{},
        mDrawActions{},
        mSprites{},
        mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
    {
        auto fb = FileBufferFactory::CreateFileBuffer(hotspotRef.ToFilename());
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

        mDrawActions.emplace_back(
            SceneSprite{
                0,
                glm::vec3{0},
                glm::vec3{1}});

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
                mDrawActions.emplace_back(
                    std::visit(overloaded{
                        [&](const BAK::DrawSprite& sa) -> DrawingAction {
                            return ConvertSceneAction(
                                sa,
                                textures,
                                offsets);
                        },
                        [](auto&& a) -> DrawingAction {
                            return ConvertSceneAction(a);
                        }
                    },
                    action));
            }
        }

        mSprites.LoadTexturesGL(textures);
    }

    BAK::SceneHotspots mHotspots;
    std::vector<DrawingAction> mDrawActions;
    Graphics::Sprites mSprites;
    const Logging::Logger& mLogger;
};

}
