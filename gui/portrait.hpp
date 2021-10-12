#pragma once

#include "bak/coordinates.hpp"
#include "bak/hotspot.hpp"
#include "bak/scene.hpp"
#include "bak/sceneData.hpp"
#include "bak/textureFactory.hpp"

#include "graphics/IGuiElement.hpp"
#include "graphics/texture.hpp"
#include "graphics/sprites.hpp"

#include "gui/IDialogScene.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/actors.hpp"
#include "gui/colors.hpp"
#include "gui/label.hpp"
#include "gui/clickButton.hpp"
#include "gui/widget.hpp"
#include "gui/scene.hpp"
#include "gui/skills.hpp"

#include "xbak/RequestResource.h"

#include <glm/glm.hpp>

#include <iostream>
#include <utility>
#include <variant>

namespace Gui {

class Portrait : public Widget
{
public:
    Portrait(
        glm::vec2 pos,
        glm::vec2 dims,
        const Actors& actors,
        const Font& font,
        Graphics::SpriteSheetIndex spriteSheet,
        Graphics::TextureIndex verticalBorder,
        Graphics::TextureIndex horizontalBorder,
        std::function<void()>&& onLeftMousePress,
        std::function<void()>&& onRightMousePress)
    :
        Widget{
            RectTag{},
            pos,
            dims,
            Color::infoBackground,
            true
        },
        mActors{actors},
        mFont{font},
        mClickButton{
            glm::vec2{0},
            dims,
            std::move(onLeftMousePress),
            std::move(onRightMousePress)},
        mClipRegion{
            ClipRegionTag{},
            glm::vec2{0},
            dims,
            true},
        mPortrait{
            ImageTag{},
            mActors.GetSpriteSheet(),
            Graphics::TextureIndex{0},
            glm::vec2{0},
            dims,
            true},
        mLeftBorder{
            ImageTag{},
            spriteSheet,
            verticalBorder,
            glm::vec2{0},
            glm::vec2{2, 72},
            true},
        mRightBorder{
            ImageTag{},
            spriteSheet,
            verticalBorder,
            glm::vec2{70, 0},
            glm::vec2{2, 72},
            true},
        mTopBorder{
            ImageTag{},
            spriteSheet,
            horizontalBorder,
            glm::vec2{2, 0},
            glm::vec2{222, 2},
            true},
        mBottomBorder{
            ImageTag{},
            spriteSheet,
            horizontalBorder,
            glm::vec2{1, 71},
            glm::vec2{222, 2},
            true},
        mLabel{
            glm::vec2{35, 72},
            dims,
            font,
            "#None"},
        mLogger{Logging::LogState::GetLogger("Gui::Portrait")}
    {
        AddChildBack(&mClickButton);

        AddChildBack(&mClipRegion);
        mClipRegion.AddChildBack(&mPortrait);
        mClipRegion.AddChildBack(&mLeftBorder);
        mClipRegion.AddChildBack(&mRightBorder);
        mClipRegion.AddChildBack(&mTopBorder);
        mClipRegion.AddChildBack(&mBottomBorder);

        AddChildBack(&mLabel);
        mLabel.SetCenter(glm::vec2{35, 72});
    }

    void SetCharacter(unsigned character, std::string_view name)
    {
        mLogger.Debug() << "Setting char: " << character << " " << name <<"\n";
        mLabel.SetText(name);
        const auto& [texture, dims] = mActors.GetActor(character + 1);
        mPortrait.SetTexture(texture);
    }

private:
    const Actors& mActors;
    const Font& mFont;

    ClickButtonBase mClickButton;

    Widget mClipRegion;

    Widget mPortrait;

    Widget mLeftBorder;
    Widget mRightBorder;
    Widget mTopBorder;
    Widget mBottomBorder;

    Label mLabel;

    const Logging::Logger& mLogger;
};

}
