#pragma once

#include "bak/scene/sceneData.hpp"

#include "graphics/texture.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace BAK {

class Image;
class Palette;

enum class Layer
{
    Flip       = 0,
    Screen     = 1,
    Background = 2,
    Save       = 3
};

Layer LayerFromArgument(unsigned arg);

// Used to render TTM scenes
class SpriteRenderer
{
public:
    static constexpr unsigned sScreenWidth = 320;
    static constexpr unsigned sScreenHeight = 200;

    SpriteRenderer();

    void SetColors(std::uint8_t fg, std::uint8_t bg);

    void RenderSprite(
        const Image& sprite,
        const Palette& palette,
        glm::ivec2 pos,
        bool flipX,
        bool flipY,
        Graphics::Texture& target);

    void RenderSpriteRotated(
        const Image& sprite,
        const Palette& palette,
        glm::ivec2 center,
        glm::ivec2 dims,
        float angle,
        Graphics::Texture& target);

    void DrawRect(
        glm::ivec2 pos,
        glm::ivec2 dims,
        const Palette& palette,
        bool filled,
        Graphics::Texture& target);

    void CopyImage(
        const Image& source,
        const Palette& palette,
        glm::ivec2 pos,
        Graphics::Texture& target);

    void CopyRect(
        const Graphics::Texture& source,
        glm::ivec2 pos,
        Graphics::Texture& target);

    Graphics::Texture ExtractRegion(glm::ivec2 pos, glm::ivec2 dims, Layer source) const;

    void CopyRect(glm::ivec2 pos, glm::ivec2 dims, Layer source, Layer target);

    Graphics::Texture& GetLayer(Layer layer);
    const Graphics::Texture& GetLayer(Layer layer) const;

    void SetClipRegion(ClipRegion clipRegion);
    void ClearClipRegion();

private:
    void SetPixel(glm::ivec2 pos, glm::vec4 color, Graphics::Texture& target);

    std::array<Graphics::Texture, 4> mLayers;
    std::optional<ClipRegion> mClipRegion;
    std::uint8_t mForegroundColor{};
    std::uint8_t mBackgroundColor{};
};

}
