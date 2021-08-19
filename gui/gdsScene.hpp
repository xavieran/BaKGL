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
#include "gui/dialogRunner.hpp"
#include "gui/hotspot.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <variant>

namespace Gui {

class GDSScene : public Widget
{
public:

    GDSScene(
        Cursor& cursor,
        BAK::HotspotRef hotspotRef,
        Graphics::SpriteManager& spriteManager,
        DialogRunner& dialogRunner)
    :
        Widget{
            Graphics::DrawMode::Sprite,
            std::invoke([&spriteManager]{
                const auto& [sheetIndex, sprites] = spriteManager.AddSpriteSheet();
                return sheetIndex;
            }),
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
            false
        },
        mReference{hotspotRef},
        mFlavourText{BAK::KeyTarget{0x10000}},
        mSpriteSheet{GetDrawInfo().mSpriteSheet},
        mFrame{
            Graphics::DrawMode::Rect,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
           false 
        },
        mClipRegion{
            Graphics::DrawMode::ClipRegion,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
            false
        },
        mHotspots{},
        mSceneElements{},
        mDialogRunner{dialogRunner},
        mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
    {

        auto fb = FileBufferFactory::CreateFileBuffer(mReference.ToFilename());
        BAK::SceneHotspots hotspots{};
        hotspots.Load(fb);
        mFlavourText = BAK::KeyTarget{hotspots.mFlavourText};

        mLogger.Debug() << "ADS: " << hotspots.mSceneADS
            << " TTM: " << hotspots.mSceneTTM
            << " " << hotspots.mSceneIndex1
            << " " << hotspots.mSceneIndex2 << "\n";

        auto fb2 = FileBufferFactory::CreateFileBuffer(hotspots.mSceneADS);
        auto sceneIndices = BAK::LoadSceneIndices(fb2);
        auto fb3 = FileBufferFactory::CreateFileBuffer(hotspots.mSceneTTM);
        auto scenes = BAK::LoadScenes(fb3);

        const auto& scene1 = scenes[sceneIndices[hotspots.mSceneIndex1].mSceneIndex];
        const auto& scene2 = scenes[sceneIndices[hotspots.mSceneIndex2].mSceneIndex];

        auto textures = Graphics::TextureStore{};
        BAK::TextureFactory::AddScreenToTextureStore(
            textures, "DIALOG.SCX", "OPTIONS.PAL");
        const auto [x, y] = textures.GetTexture(0).GetDims();
        this->mPositionInfo.mDimensions = glm::vec2{x, y};

        std::unordered_map<unsigned, unsigned> offsets{};

        // We do some pretty wacky stuff here to end up with 
        // Background
        // -> Rect OR
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
                                glm::vec2{sr.mTopLeft.x, sr.mTopLeft.y},
                                glm::vec2{sr.mBottomRight.x, sr.mBottomRight.y}};

                            mFrame = Widget{
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
                            mClipRegion = Widget{
                                Graphics::DrawMode::ClipRegion,
                                mSpriteSheet,
                                0, // no image index
                                Graphics::ColorMode::SolidColor,
                                glm::vec4{1},
                                glm::vec2{clip.mTopLeft.x, clip.mTopLeft.y},
                                glm::vec2{clip.mDims.x, clip.mDims.y},
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


        // Want our refs to be stable..
        mHotspots.reserve(hotspots.mHotspots.size());
        for (const auto& hs : hotspots.mHotspots)
        {
            mHotspots.emplace_back(
                cursor,
                hs.mTopLeft,
                hs.mDimensions,
                hs.mKeyword - 1, // cursor index
                [this, hs](){
                    HandleHotspotClicked(hs);
                },
                [](){});

            AddChildBack(
                &mHotspots.back());
        }
    }

    void HandleHotspotClicked(const BAK::Hotspot& hotspot)
    {
        Logging::LogDebug("Gui::GDSScene") << "Hotspot: " << hotspot << "\n";
        mDialogRunner.BeginDialog(
            hotspot.mTooltip);

        /*if (hs.mAction == BAK::HotspotAction::GOTO)
        {
            char c = static_cast<char>(65 + arg - 1);
            currentSceneRef = BAK::HotspotRef{root, c};
            logger.Debug() << "Switching to: " << c << " " 
                << currentSceneRef.ToFilename() << "\n";
        }
        else if (hs.mAction == BAK::HotspotAction::EXIT)
        {
            if (scenes.size() > 1)
            {
                scenes.pop();
                frames.pop();
                currentSceneRef = scenes.top()->mReference;
            }
            else
            {
                std::exit(0);
            }
        }*/
    }

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

    BAK::HotspotRef mReference;
    BAK::Target mFlavourText;

    Graphics::SpriteSheetIndex mSpriteSheet;
    Widget mFrame;
    Widget mClipRegion;

    std::vector<Hotspot> mHotspots;
    std::vector<Widget> mSceneElements;
    DialogRunner& mDialogRunner;

    const Logging::Logger& mLogger;
};

}
