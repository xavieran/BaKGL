#pragma once

#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/texture.hpp"

#include "graphics/texture.hpp"

#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <variant>

namespace Gui {

class GDSScene
{
public:

    GDSScene(HotspotRef hotspotRef)
    {
        auto fb = FileBufferFactory::CreateFileBuffer(argv[1]);
        mHotspots.Load(fb);

        logger.Debug() << "ADS: " << hotspots.mSceneADS
            << " TTM: " << hotspots.mSceneTTM
            << " " << hotspots.mSceneIndex1
            << " " << hotspots.mSceneIndex2 << "\n";

        auto fb2 = FileBufferFactory::CreateFileBuffer(hotspots.mSceneADS);
        auto sceneIndices = BAK::LoadSceneIndices(fb2);
        auto fb3 = FileBufferFactory::CreateFileBuffer(hotspots.mSceneTTM);
        auto scenes = BAK::LoadScenes(fb3);

        const auto& scene1 = scenes[sceneIndices[hotspots.mSceneIndex1].mSceneIndex];
        const auto& scene2 = scenes[sceneIndices[hotspots.mSceneIndex2].mSceneIndex];

        BAK::TextureFactory::AddScreenToTextureStore(
            mTextures, "DIALOG.SCX", "OPTIONS.PAL");

        std::unordered_map<unsigned, unsigned> offsets{};

        mDrawActions.emplace_back(
            Gui::SceneSprite{
                0,
                glm::vec3{0},
                glm::vec3{1}});

        for (const auto& scene : {scene1, scene2})
        {
            for (const auto& [imageKey, imagePal] : scene.mImages)
            {
                const auto& [image, palKey] = imagePal;
                const auto& palette = scene.mPalettes.find(palKey)->second;
                offsets[imageKey] = mTextures.GetTextures().size();
                BAK::TextureFactory::AddToTextureStore(
                    mTextures,
                    image,
                    palette);
            }

            for (const auto& action : scene.mActions)
            {
                mDrawActions.emplace_back(
                    std::visit(overloaded{
                        [&](const BAK::DrawSprite& sa) -> Gui::DrawingAction {
                            return Gui::ConvertSceneAction(
                                sa,
                                mTextures,
                                offsets);
                        },
                        [](auto&& a) -> Gui::DrawingAction {
                            return Gui::ConvertSceneAction(a);
                        }
                    },
                    action));
            }
        }
    }

    BAK::SceneHotspots mHotspots;
    std::vector<Gui::DrawingAction> mDrawActions;
    Sprites mSprites;
};

}
