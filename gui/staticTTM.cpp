#include "gui/staticTTM.hpp"

#include "bak/textureFactory.hpp"

#include "com/logger.hpp"

#include "graphics/guiTypes.hpp"
#include "graphics/sprites.hpp"
#include "graphics/texture.hpp"
#include "graphics/types.hpp"

#include "gui/colors.hpp"
#include "gui/scene.hpp"
#include "gui/widget.hpp"

namespace Gui {

StaticTTM::StaticTTM(
    Graphics::SpriteManager& spriteManager,
    const BAK::Scene& sceneInit,
    const BAK::Scene& sceneContent)
:
    mSpriteSheet{spriteManager.AddSpriteSheet()},
    mSceneFrame{
        Graphics::DrawMode::Rect,
        mSpriteSheet,
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
    mLogger.Debug() << "Loading scene: " << sceneInit << " with " << sceneContent << "\n";
    auto textures = Graphics::TextureStore{};
    std::unordered_map<unsigned, unsigned> offsets{};

    // Load all the image slots
    for (const auto& scene : {sceneInit, sceneContent})
    {
        for (const auto& [imageKey, imagePal] : scene.mImages)
        {
            const auto& [image, palKey] = imagePal;
            mLogger.Debug() << "Loading image slot: " << imageKey 
                << " (" << image << ")\n";
            const auto& palette = scene.mPalettes.find(palKey)->second;
            offsets[imageKey] = textures.GetTextures().size();

            BAK::TextureFactory::AddToTextureStore(
                textures,
                image,
                palette);
        }
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
            mSpriteSheet,
            texture,
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
            false 
        );
    }

    // Add widgets for each scene action
    for (const auto& scene : {sceneInit, sceneContent})
    {
        for (const auto& action : scene.mActions)
        {
            std::visit(
                overloaded{
                    [&](const BAK::DrawSprite& sa){
                        const auto sceneSprite = ConvertSceneAction(
                            sa,
                            textures,
                            offsets);

                        auto& elem = mSceneElements.emplace_back(
                            Graphics::DrawMode::Sprite,
                            mSpriteSheet,
                            sceneSprite.mImage,
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
                        constexpr auto FRAME_COLOR_INDEX = 6;
                        const auto [palKey, colorKey] = sr.mPaletteColor;
                        const auto sceneRect = SceneRect{
                            // Reddy brown frame color
                            colorKey == FRAME_COLOR_INDEX
                                ? Gui::Color::frameMaroon
                                : Gui::Color::black,
                            glm::vec2{sr.mTopLeft.x, sr.mTopLeft.y},
                            // FIXME: This should be renamed dims not bottom right...
                            glm::vec2{sr.mBottomRight.x, sr.mBottomRight.y}};
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
                        const auto clip = ConvertSceneAction(a);
                        mClipRegion.emplace(
                            Graphics::DrawMode::ClipRegion,
                            mSpriteSheet,
                            0, // no image index
                            Graphics::ColorMode::SolidColor,
                            glm::vec4{1},
                            clip.mTopLeft,
                            clip.mDims,
                            false);
                        mSceneFrame.AddChildBack(&(*mClipRegion));
                    },
                    [&](const BAK::DisableClipRegion&){
                        // Doesn't really do anything...
                        // in the future maybe pop the clip region
                        // so we could add another one?
                    }
                },
                action
            );
        }
    }

    spriteManager
        .GetSpriteSheet(mSpriteSheet)
        .LoadTexturesGL(textures);
}

Widget* StaticTTM::GetScene()
{
    return &mSceneFrame;
}

Widget* StaticTTM::GetBackground()
{
    assert(mDialogBackground);
    return &(*mDialogBackground);
}

}
