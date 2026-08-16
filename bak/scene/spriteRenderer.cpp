#include "bak/scene/spriteRenderer.hpp"

#include "bak/image.hpp"
#include "bak/palette.hpp"

#include "bak/scene/sceneData.hpp"

#include "graphics/texture.hpp"

#include <optional>

namespace BAK {

SpriteRenderer::SpriteRenderer()
:
    mForegroundLayer{320, 200, 320, 200},
    mBackgroundLayer{320, 200, 320, 200},
    mSavedImagesLayer0{320, 200, 320, 200},
    mSavedImagesLayer1{320, 200, 320, 200},
    mSavedImagesLayerBG{320, 200, 320, 200}
{
}

void SpriteRenderer::SetColors(std::uint8_t fg, std::uint8_t bg)
{
    mForegroundColor = fg;
    mBackgroundColor = bg;
}

void SpriteRenderer::SetPixel(
    glm::ivec2 pos,
    glm::vec4 color,
    Graphics::Texture& layer)
{
    if (color.a == 0) return;

    if (mClipRegion
        && (pos.x < mClipRegion->mTopLeft.x || pos.x > mClipRegion->mBottomRight.x
            || pos.y < mClipRegion->mTopLeft.y || pos.y > mClipRegion->mBottomRight.y))
    {
        return;
    }

    if (pos.x < 0 || pos.x >= static_cast<int>(layer.GetWidth())
        || pos.y < 0 || pos.y >= static_cast<int>(layer.GetHeight()))
    {
        return;
    }

    layer.SetPixel(pos.x, pos.y, color);
}

void SpriteRenderer::RenderTexture(
    const Graphics::Texture& texture,
    glm::ivec2 pos,
    Graphics::Texture& layer)
{
    for (int x = 0; x < static_cast<int>(texture.GetWidth()); x++)
    {
        for (int y = 0; y < static_cast<int>(texture.GetHeight()); y++)
        {
            SetPixel(pos + glm::ivec2{x, y}, texture.GetPixel(x, y), layer);
        }
    }
}

void SpriteRenderer::RenderSprite(
    BAK::Image sprite,
    const BAK::Palette& palette,
    glm::ivec2 pos,
    bool flipX,
    bool flipY,
    Graphics::Texture& layer)
{
    const auto width  = static_cast<int>(sprite.GetWidth());
    const auto height = static_cast<int>(sprite.GetHeight());

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            const auto pixelPos = pos + glm::ivec2{
                flipX ? width  - 1 - x : x,
                flipY ? height - 1 - y : y};

            SetPixel(pixelPos, palette.GetColor(sprite.GetPixel(x, y)), layer);
        }
    }
}

void SpriteRenderer::DrawRect(
    glm::ivec2 pos,
    glm::ivec2 dims,
    const BAK::Palette& palette,
    bool filled,
    Graphics::Texture& layer)
{
    if (dims.x <= 0 || dims.y <= 0) return;

    const auto right  = pos.x + dims.x - 1;
    const auto bottom = pos.y + dims.y - 1;

    if (filled)
    {
        const auto fill = palette.GetColor(mBackgroundColor);
        for (int y = pos.y; y <= bottom; y++)
        {
            for (int x = pos.x; x <= right; x++)
            {
                SetPixel(glm::ivec2{x, y}, fill, layer);
            }
        }
    }

    const auto edge = palette.GetColor(mForegroundColor);
    for (int x = pos.x; x <= right; x++)
    {
        SetPixel(glm::ivec2{x, pos.y}, edge, layer);
        SetPixel(glm::ivec2{x, bottom}, edge, layer);
    }
    for (int y = pos.y; y <= bottom; y++)
    {
        SetPixel(glm::ivec2{pos.x, y}, edge, layer);
        SetPixel(glm::ivec2{right, y}, edge, layer);
    }
}

void SpriteRenderer::Clear()
{
    mForegroundLayer = Graphics::Texture{320, 200, 320, 200};
    mBackgroundLayer = Graphics::Texture{320, 200, 320, 200};
    mSavedImagesLayer0 = Graphics::Texture{320, 200, 320, 200};
    mSavedImagesLayer1 = Graphics::Texture{320, 200, 320, 200};
    mSavedImagesLayerBG = Graphics::Texture{320, 200, 320, 200};
}

Graphics::Texture& SpriteRenderer::GetForegroundLayer()
{
    return mForegroundLayer;
}

Graphics::Texture& SpriteRenderer::GetBackgroundLayer()
{
    return mBackgroundLayer;
}

Graphics::Texture& SpriteRenderer::GetSavedImagesLayerBG()
{
    return mSavedImagesLayerBG;
}

Graphics::Texture& SpriteRenderer::GetSavedImagesLayer0()
{
    return mSavedImagesLayer0;
}

Graphics::Texture& SpriteRenderer::GetSavedImagesLayer1()
{
    return mSavedImagesLayer1;
}

void SpriteRenderer::SetClipRegion(BAK::ClipRegion clipRegion)
{
    mClipRegion = clipRegion;
}

void SpriteRenderer::ClearClipRegion()
{
    mClipRegion.reset();
}

Graphics::Texture SpriteRenderer::SaveImage(glm::ivec2 pos, glm::ivec2 dims, unsigned layer)
{
    auto image = Graphics::Texture{
        static_cast<unsigned>(dims.x),
        static_cast<unsigned>(dims.y),
        static_cast<unsigned>(dims.x),
        static_cast<unsigned>(dims.y)};
    for (int x = 0; x < dims.x; x++)
    {
        for (int y = 0; y < dims.y; y++)
        {
            image.SetPixel(x, y, mForegroundLayer.GetPixel(x + pos.x, y + pos.y));
        }
    }

    RenderTexture(image, pos, GetSaveLayer(layer));
    return image;
}

Graphics::Texture& SpriteRenderer::GetSaveLayer(unsigned layer)
{
    if (layer == 0)
    {
        return mSavedImagesLayer0;
    }
    else if (layer == 1)
    {
        return mSavedImagesLayer1;
    }
    else if (layer == 2)
    {
        return mSavedImagesLayerBG;
    }
    assert(false);
    return mSavedImagesLayer0;
}

}
