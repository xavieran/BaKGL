#pragma once

#include "bak/palette.hpp"
#include "bak/sceneData.hpp"

#include "graphics/texture.hpp"

#include <optional>

namespace BAK {

class Image;
class Palette;

// Used to render TTM scenes
class SpriteRenderer
{
public:
    SpriteRenderer();

    void SetColors(std::uint8_t fg, std::uint8_t bg);

    void RenderTexture(
        const Graphics::Texture& texture,
        glm::ivec2 pos,
        Graphics::Texture& layer);

    void RenderSprite(
        Image sprite,
        const Palette& palette,
        glm::ivec2 pos,
        bool flipped,
        Graphics::Texture& layer);

    void DrawRect(
        glm::ivec2 pos,
        glm::ivec2 dims,
        const Palette& palette,
        Graphics::Texture& layer);

    Graphics::Texture SaveImage(glm::ivec2 pos, glm::ivec2 dims, unsigned layer);
    Graphics::Texture& GetSaveLayer(unsigned layer);

    void Clear();
    Graphics::Texture& GetForegroundLayer();
    Graphics::Texture& GetBackgroundLayer();
    Graphics::Texture& GetSavedImagesLayerBG();
    Graphics::Texture& GetSavedImagesLayer0();
    Graphics::Texture& GetSavedImagesLayer1();
    void SetClipRegion(ClipRegion clipRegion);
    void ClearClipRegion();

private:
    Graphics::Texture mForegroundLayer;
    Graphics::Texture mBackgroundLayer;
    Graphics::Texture mSavedImagesLayer0;
    Graphics::Texture mSavedImagesLayer1;
    Graphics::Texture mSavedImagesLayerBG;
    std::optional<ClipRegion> mClipRegion;
    std::uint8_t mBackgroundColor{};
    std::uint8_t mForegroundColor{};
};

}
