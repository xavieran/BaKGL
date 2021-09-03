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
            spriteManager.AddSpriteSheet(),
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
        mClipRegion{},
        mSceneFrame{
            Graphics::DrawMode::Rect,
            0,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::SolidColor,
            glm::vec4{0},
            glm::vec2{0},
            glm::vec2{1},
            false 
        },
        mBackgroundFrame{
            Graphics::DrawMode::Sprite,
            mSpriteSheet,
            Graphics::TextureIndex{0},
            Graphics::ColorMode::Texture,
            glm::vec4{1},
            glm::vec2{0},
            glm::vec2{1},
            false 
        },
        mMousePos{glm::vec2{0}},
        mHotspots{},
        mSceneElements{},
        mCursor{cursor},
        mDialogRunner{dialogRunner},
        mDialogState{false, false, glm::vec2{0}},
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
        SetDimensions(glm::vec2{x, y});

        std::unordered_map<unsigned, unsigned> offsets{};

        for (const auto& scene : {scene1, scene2})
        {
            for (const auto& [imageKey, imagePal] : scene.mImages)
            {
                const auto& [image, palKey] = imagePal;
                mLogger.Debug() << "Loading image slot: " << imageKey << " (" << image << ")\n";
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
                            // DialogBackground will have same dims...
                            mBackgroundFrame.SetPosition(sceneRect.mPosition);
                            mBackgroundFrame.SetDimensions(sceneRect.mDimensions);
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
                            this->AddChildBack(&(*mClipRegion));
                        },
                        [&](const BAK::DisableClipRegion&){

                        }
                    },
                    action);
            }
        }

        for (auto& element : mSceneElements)
        {
            mSceneFrame.AddChildBack(&element);
        }

        const auto dialogBackground = offsets.find(5);
        if (dialogBackground != offsets.end())
        {
            mBackgroundFrame.SetTexture(dialogBackground->second);
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
                    HandleHotspotLeftClicked(hs);
                },
                [this, hs](){
                    HandleHotspotRightClicked(hs);
                });

            if (mClipRegion)
                mClipRegion->AddChildBack(
                    &mHotspots.back());
            else
                mFrame.AddChildBack(
                    &mHotspots.back());
        }

        spriteManager.GetSpriteSheet(mSpriteSheet).LoadTexturesGL(textures);

        SetDisplayScene();
    }

    void SetDisplayScene()
    {
        Graphics::IGuiElement* addTo = GetChildren().size() > 1
            ? GetChildren().back()
            : this;
        const auto scenePtr = std::find(addTo->GetChildren().begin(), addTo->GetChildren().end(), &mBackgroundFrame);
        if (scenePtr != addTo->GetChildren().end())
            addTo->RemoveChild(&mBackgroundFrame);
        addTo->AddChildBack(&mSceneFrame);
    }

    void SetDisplayBackground()
    {
        mLogger.Debug() << "Set to background\n";
        Graphics::IGuiElement* addTo = GetChildren().size() > 1
            ? GetChildren().back()
            : this;

        const auto scenePtr = std::find(
            addTo->GetChildren().begin(),
            addTo->GetChildren().end(),
            &mSceneFrame);

        if (scenePtr != addTo->GetChildren().end())
            addTo->RemoveChild(&mSceneFrame);

        addTo->AddChildBack(&mBackgroundFrame);
    }

    void LeftMousePress(glm::vec2 pos) override
    {
        if (mDialogState.mDialogActive)
        {
            const auto dialogActive = mDialogState.RunDialog([&]{
                return mDialogRunner.RunDialog();
            });

            if (!dialogActive)
            {
                mLogger.Debug() << "Dialog finished, back to flavour text\n";
                mDialogRunner.ShowFlavourText(mFlavourText);
                mCursor.Show();
            }
        }
        // Only propagate event if no dialog active
        else
        {
            Widget::LeftMousePress(pos);
        }

    }

    void HandleHotspotLeftClicked(const BAK::Hotspot& hotspot)
    { 
        mLogger.Debug() << "Hotspot: " << hotspot << "\n";

        if (hotspot.mAction == BAK::HotspotAction::DIALOG)
        {
            mDialogState.ActivateDialog();
            mDialogRunner.BeginDialog(
                BAK::KeyTarget{hotspot.mActionArg2});
            mCursor.Hide();
        }
    }

    void HandleHotspotRightClicked(const BAK::Hotspot& hotspot)
    {
        mLogger.Debug() << "Hotspot: " << hotspot << "\n";
        if (!mDialogState.mDialogActive)
        {
            mDialogRunner.BeginDialog(hotspot.mTooltip);
            mDialogState.ActivateTooltip(mMousePos);
            mCursor.Hide();
        }
    }

    void MouseMoved(glm::vec2 pos)
    {
        mMousePos = pos;

        mDialogState.DeactivateTooltip(
            pos,
            [&]{
                mDialogRunner.ShowFlavourText(mFlavourText);
                mCursor.Show();
            }
        );
        
        if (!mDialogState.mDialogActive)
            Widget::MouseMoved(pos);
    }

    GDSScene(const GDSScene&) = delete;
    GDSScene& operator=(const GDSScene&) = delete;

public:
//private:
    struct DialogState
    {
        void ActivateDialog()
        {
            mDialogActive = true;
        }

        template <typename F>
        bool RunDialog(F&& f)
        {
            if (mDialogActive)
            {
                mDialogActive = f();
            }
            return mDialogActive;
        }

        void ActivateTooltip(glm::vec2 pos)
        {
            mTooltipPos = pos;
            mTooltipActive = true;
        }

        template <typename F>
        void DeactivateTooltip(glm::vec2 pos, F&& f)
        {
            constexpr auto tooltipSensitivity = 20;
            if (mTooltipActive 
                && glm::distance(pos, mTooltipPos) > tooltipSensitivity)
            {
                f();
                mTooltipActive = false;
            }
        }

        bool mDialogActive;
        bool mTooltipActive;
        glm::vec2 mTooltipPos;
    };

    BAK::HotspotRef mReference;
    BAK::Target mFlavourText;

    Graphics::SpriteSheetIndex mSpriteSheet;
    Widget mFrame;
    std::optional<Widget> mClipRegion;
    Widget mSceneFrame;
    Widget mBackgroundFrame;
    glm::vec2 mMousePos;

    std::vector<Hotspot> mHotspots;
    std::vector<Widget> mSceneElements;

    Cursor& mCursor;
    DialogRunner& mDialogRunner;
    DialogState mDialogState;

    const Logging::Logger& mLogger;
};

}
