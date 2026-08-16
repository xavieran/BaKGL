#include "gui/staticTTM.hpp"

#include "bak/palette.hpp"
#include "bak/textureFactory.hpp"
#include "bak/scene/scene.hpp"
#include "bak/scene/sceneData.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include "gui/colors.hpp"
#include "gui/scene.hpp"

#include "graphics/types.hpp"

namespace Gui {

StaticTTM::StaticTTM(
    Graphics::SpriteManager& spriteManager,
    const BAK::Script& sceneInit,
    const BAK::Script& sceneContent)
:
    mSpriteSheet{spriteManager.AddTemporarySpriteSheet()},
    mSceneFrame{
        Graphics::DrawMode::Rect,
        mSpriteSheet->mSpriteSheet,
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
    mLogger{Logging::LogState::GetLogger("Gui::StaticTTM")}
{
    mLogger.Debug() << "Loading scene: " << sceneInit << " with " << sceneContent << std::endl;
    auto textures = Graphics::TextureStore{};
    std::unordered_map<unsigned, unsigned> offsets{};

    constexpr auto SCENE_PALETTE_SLOT = 0;
    constexpr auto ACTOR_IMAGE_SLOT = 1;
    std::optional<BAK::Palette> scenePalette{};
    std::optional<std::string> scenePaletteName{};
    std::optional<std::string> actorImageName{};

    // Load all the image slots
    for (const auto& scene : {sceneInit, sceneContent})
    {
        const auto scenePal = scene.mPalettes.find(SCENE_PALETTE_SLOT);
        if (scenePal != scene.mPalettes.end())
        {
            scenePalette.emplace(scenePal->second);
            scenePaletteName = scenePal->second;
        }

        for (const auto& [imageKey, imagePal] : scene.mImages)
        {
            const auto& [image, palKey] = imagePal;
            if (imageKey == ACTOR_IMAGE_SLOT)
            {
                actorImageName = image;
            }
            assert(scene.mPalettes.find(palKey) != scene.mPalettes.end());
            const auto& palette = scene.mPalettes.find(palKey)->second;
            mLogger.Debug() << "Loading image slot: " << imageKey 
                << " (" << image << ") with palette: " << palKey << std::endl;
            offsets[imageKey] = textures.GetTextures().size();

            BAK::TextureFactory::AddToTextureStore(
                textures,
                image,
                palette);
        }
        for (const auto& [screenKey, screenPal] : scene.mScreens)
        {
            const auto& [screen, palKey] = screenPal;
            const auto& palette = scene.mPalettes.find(palKey)->second;
            mLogger.Debug() << "Loading screen slot: " << screenKey 
                << " (" << screen << ") with palette: " << palKey << std::endl;
            offsets[25] = textures.GetTextures().size();

            BAK::TextureFactory::AddScreenToTextureStore(
                textures,
                screen,
                palette);
        }
    }

    std::optional<unsigned> actorSprite{};
    if (actorImageName && scenePaletteName)
    {
        actorSprite = textures.GetTextures().size();
        BAK::TextureFactory::AddToTextureStore(
            textures,
            *actorImageName,
            *scenePaletteName);
    }

    // Make sure all the refs are constant
    mSceneElements.reserve(
        sceneInit.mActions.size()
        + sceneContent.mActions.size());

    // Some scenes will have a dialog background specified in slot 5
    // Find and add it if so
    constexpr auto DIALOG_BACKGROUND_SLOT = 5;
    const auto dialogBackground = offsets.find(DIALOG_BACKGROUND_SLOT);
    if (dialogBackground != offsets.end())
    {
        const auto texture = dialogBackground->second;
        mDialogBackground.emplace(
            Graphics::DrawMode::Sprite,
            mSpriteSheet->mSpriteSheet,
            Graphics::TextureIndex{texture},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
            false 
        );
    }

    for (const auto& scene : {sceneInit, sceneContent})
    {
        for (const auto& action : scene.mActions)
        {
            mLogger.Debug() << " Action: " << action << "\n";
            std::visit(
                overloaded{
                    [&](const BAK::DrawSprite& sa){
                        const auto sceneSprite = ConvertScriptAction(
                            sa,
                            textures,
                            offsets);

                        auto& elem = mSceneElements.emplace_back(
                            Graphics::DrawMode::Sprite,
                            mSpriteSheet->mSpriteSheet,
                            Graphics::TextureIndex{sceneSprite.mImage},
                            Graphics::ColorMode::Texture,
                            glm::vec4{1},
                            sceneSprite.mPosition,
                            sceneSprite.mScale,
                            false);

                        // Either add to the clip region or to the frame
                        // This doesn't work if the scene has more than one
                        // clip region...
                        if (mClipRegion)
                            mClipRegion->AddChildBack(&elem);
                        else
                            mSceneFrame.AddChildBack(&elem);
                    },
                    [&](const BAK::DrawRect& sr){
                        const auto frameColor = scenePalette
                            ? glm::vec4{
                                glm::vec3{scenePalette->GetColor(sr.mEdgeColor)},
                                1}
                            : Gui::Color::black;
                        const auto sceneRect = SceneRect{
                            frameColor,
                            glm::vec2{sr.mPos.x, sr.mPos.y},
                            glm::vec2{sr.mDims.x, sr.mDims.y}};
                        // This really only works for "DrawFrame", not "DrawRect"
                        mSceneFrame.SetPosition(sceneRect.mPosition);
                        mSceneFrame.SetDimensions(sceneRect.mDimensions);
                        mSceneFrame.SetColor(sceneRect.mColor);

                        // DialogBackground will have same dims...
                        if (mDialogBackground)
                        {
                            mDialogBackground->SetPosition(sceneRect.mPosition + glm::vec2{1,1});
                            mDialogBackground->SetDimensions(sceneRect.mDimensions - glm::vec2{2, 2});
                        }
                    },
                    [&](const BAK::ClipRegion& a){
                        const auto clip = ConvertScriptAction(a);
                        mClipRegion.emplace(
                            ClipRegionTag{},
                            clip.mTopLeft,
                            clip.mDims,
                            false);
                        mSceneFrame.AddChildBack(&(*mClipRegion));
                    },
                    [&](const BAK::DisableClipRegion&){
                        // Doesn't really do anything...
                        // in the future maybe pop the clip region
                        // so we could add another one?
                    },
                    [&](const BAK::ShowDialog& sd){
                        // ShowDialog with key -1 draws the actor image
                        // rather than showing a dialog. Seems to be
                        // restricted to static TTMs.
                        if (sd.mDialogKey || !actorSprite)
                            return;
                        const auto sprite = *actorSprite;
                        const auto tex = textures.GetTexture(sprite);
                        const auto width = static_cast<int>(tex.GetTargetWidth());
                        const auto height = static_cast<int>(tex.GetTargetHeight());

                        auto& elem = mSceneElements.emplace_back(
                            Graphics::DrawMode::Sprite,
                            mSpriteSheet->mSpriteSheet,
                            Graphics::TextureIndex{sprite},
                            Graphics::ColorMode::Texture,
                            glm::vec4{1},
                            glm::vec2{160 - width / 2, 112 - height},
                            glm::vec2{width, height},
                            false);

                        if (mClipRegion)
                            mClipRegion->AddChildBack(&elem);
                        else
                            mSceneFrame.AddChildBack(&elem);
                    },
                    [&](const auto&){}
                },
                action
            );
        }
    }

    spriteManager
        .GetSpriteSheet(mSpriteSheet->mSpriteSheet)
        .LoadTexturesGL(textures);
}

Widget* StaticTTM::GetScene()
{
    return &mSceneFrame;
}

Widget* StaticTTM::GetBackground()
{
    ASSERT(mDialogBackground);
    return &(*mDialogBackground);
}

}
