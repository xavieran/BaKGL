#include "bak/scene/spriteRenderer.hpp"

#include "bak/image.hpp"
#include "bak/palette.hpp"

#include "bak/scene/sceneData.hpp"

#include "graphics/texture.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <vector>

namespace BAK {

Layer LayerFromArgument(unsigned arg)
{
    switch (arg)
    {
    case 0: return Layer::Flip;
    case 1: return Layer::Screen;
    case 2: return Layer::Background;
    case 3: return Layer::Save;
    default: return Layer::Screen;
    }
}

SpriteRenderer::SpriteRenderer()
:
    mLayers{{
        {sScreenWidth, sScreenHeight, sScreenWidth, sScreenHeight},
        {sScreenWidth, sScreenHeight, sScreenWidth, sScreenHeight},
        {sScreenWidth, sScreenHeight, sScreenWidth, sScreenHeight},
        {sScreenWidth, sScreenHeight, sScreenWidth, sScreenHeight}}}
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
    Graphics::Texture& target)
{
    if (mClipRegion
        && (pos.x < mClipRegion->mTopLeft.x || pos.x > mClipRegion->mBottomRight.x
            || pos.y < mClipRegion->mTopLeft.y || pos.y > mClipRegion->mBottomRight.y))
    {
        return;
    }

    if (pos.x < 0 || pos.x >= static_cast<int>(target.GetWidth())
        || pos.y < 0 || pos.y >= static_cast<int>(target.GetHeight()))
    {
        return;
    }

    target.SetPixel(pos.x, pos.y, color);
}

void SpriteRenderer::RenderSprite(
    const Image& sprite,
    const Palette& palette,
    glm::ivec2 pos,
    bool flipX,
    bool flipY,
    Graphics::Texture& target)
{
    const auto width  = static_cast<int>(sprite.GetWidth());
    const auto height = static_cast<int>(sprite.GetHeight());

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            const auto index = sprite.GetPixel(x, y);
            if (index == 0) continue;

            const auto pixelPos = pos + glm::ivec2{
                flipX ? width  - 1 - x : x,
                flipY ? height - 1 - y : y};

            SetPixel(pixelPos, palette.GetColor(index), target);
        }
    }
}

namespace {

int ScaleCoordinate(int destIndex, int destSteps, int sourceMax, bool flip)
{
    const auto start = flip ? static_cast<float>(sourceMax) : 0.f;
    const auto end   = flip ? 0.f : static_cast<float>(sourceMax);
    const auto step  = destSteps > 0 ? (end - start) / destSteps : 0.f;
    const auto coord = static_cast<int>(std::floor(start + step * destIndex));
    return std::clamp(coord, 0, sourceMax);
}

}

void SpriteRenderer::RenderSpriteScaled(
    const Image& sprite,
    const Palette& palette,
    glm::ivec2 pos,
    glm::ivec2 dims,
    bool flipX,
    bool flipY,
    Graphics::Texture& target)
{
    if (dims.x < 0)
    {
        dims.x = -dims.x;
        flipX = !flipX;
    }
    if (dims.y < 0)
    {
        dims.y = -dims.y;
        flipY = !flipY;
    }

    const auto sourceWidth  = static_cast<int>(sprite.GetWidth());
    const auto sourceHeight = static_cast<int>(sprite.GetHeight());
    if (dims.x == 0 || dims.y == 0 || sourceWidth == 0 || sourceHeight == 0) return;

    auto sourceColumns = std::vector<int>(dims.x);
    for (int x = 0; x < dims.x; x++)
    {
        sourceColumns[x] = ScaleCoordinate(x, dims.x - 1, sourceWidth - 1, flipX);
    }

    for (int y = 0; y < dims.y; y++)
    {
        const auto sourceY = ScaleCoordinate(y, dims.y - 1, sourceHeight - 1, flipY);
        for (int x = 0; x < dims.x; x++)
        {
            const auto index = sprite.GetPixel(
                static_cast<unsigned>(sourceColumns[x]),
                static_cast<unsigned>(sourceY));
            if (index == 0) continue;

            SetPixel(pos + glm::ivec2{x, y}, palette.GetColor(index), target);
        }
    }
}

void SpriteRenderer::RenderSpriteRotated(
    const Image& sprite,
    const Palette& palette,
    glm::ivec2 center,
    glm::ivec2 dims,
    float angle,
    Graphics::Texture& target)
{
    const auto width  = static_cast<float>(std::abs(dims.x));
    const auto height = static_cast<float>(std::abs(dims.y));
    if (width == 0 || height == 0) return;

    const auto cosA = std::cos(angle);
    const auto sinA = std::sin(angle);
    const auto widthAxis  = glm::vec2{cosA, sinA} * width;
    const auto heightAxis = glm::vec2{-sinA, cosA} * height;
    const auto origin = glm::vec2{center} - (widthAxis + heightAxis) * 0.5f;

    const auto corners = std::array<glm::vec2, 4>{
        origin,
        origin + widthAxis,
        origin + widthAxis + heightAxis,
        origin + heightAxis};

    auto topLeft = corners[0];
    auto bottomRight = corners[0];
    for (const auto& corner : corners)
    {
        topLeft = glm::min(topLeft, corner);
        bottomRight = glm::max(bottomRight, corner);
    }

    const auto sourceWidth  = static_cast<float>(sprite.GetWidth());
    const auto sourceHeight = static_cast<float>(sprite.GetHeight());

    const auto right  = static_cast<int>(std::ceil(bottomRight.x));
    const auto bottom = static_cast<int>(std::ceil(bottomRight.y));

    for (int y = static_cast<int>(std::floor(topLeft.y)); y <= bottom; y++)
    {
        for (int x = static_cast<int>(std::floor(topLeft.x)); x <= right; x++)
        {
            const auto offset = glm::vec2{x + 0.5f, y + 0.5f} - origin;
            const auto u = (offset.x * cosA + offset.y * sinA) / width;
            const auto v = (offset.y * cosA - offset.x * sinA) / height;
            if (u < 0.f || u >= 1.f || v < 0.f || v >= 1.f) continue;

            const auto index = sprite.GetPixel(
                static_cast<unsigned>(u * sourceWidth),
                static_cast<unsigned>(v * sourceHeight));
            if (index == 0) continue;

            SetPixel(glm::ivec2{x, y}, palette.GetColor(index), target);
        }
    }
}

void SpriteRenderer::DrawRect(
    glm::ivec2 pos,
    glm::ivec2 dims,
    const Palette& palette,
    bool filled,
    Graphics::Texture& target)
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
                SetPixel(glm::ivec2{x, y}, fill, target);
            }
        }
    }

    const auto edge = palette.GetColor(mForegroundColor);
    for (int x = pos.x; x <= right; x++)
    {
        SetPixel(glm::ivec2{x, pos.y}, edge, target);
        SetPixel(glm::ivec2{x, bottom}, edge, target);
    }
    for (int y = pos.y; y <= bottom; y++)
    {
        SetPixel(glm::ivec2{pos.x, y}, edge, target);
        SetPixel(glm::ivec2{right, y}, edge, target);
    }
}

void SpriteRenderer::CopyImage(
    const Image& source,
    const Palette& palette,
    glm::ivec2 pos,
    Graphics::Texture& target)
{
    for (unsigned y = 0; y < source.GetHeight(); y++)
    {
        for (unsigned x = 0; x < source.GetWidth(); x++)
        {
            SetPixel(
                pos + glm::ivec2{static_cast<int>(x), static_cast<int>(y)},
                palette.GetColor(source.GetPixel(x, y)),
                target);
        }
    }
}

void SpriteRenderer::CopyRect(
    const Graphics::Texture& source,
    glm::ivec2 pos,
    Graphics::Texture& target)
{
    for (unsigned y = 0; y < source.GetHeight(); y++)
    {
        for (unsigned x = 0; x < source.GetWidth(); x++)
        {
            SetPixel(
                pos + glm::ivec2{static_cast<int>(x), static_cast<int>(y)},
                source.GetPixel(x, y),
                target);
        }
    }
}

Graphics::Texture SpriteRenderer::ExtractRegion(
    glm::ivec2 pos,
    glm::ivec2 dims,
    Layer source) const
{
    const auto& layer = GetLayer(source);
    auto region = Graphics::Texture{
        static_cast<unsigned>(dims.x),
        static_cast<unsigned>(dims.y),
        static_cast<unsigned>(dims.x),
        static_cast<unsigned>(dims.y)};

    for (int y = 0; y < dims.y; y++)
    {
        for (int x = 0; x < dims.x; x++)
        {
            const auto src = pos + glm::ivec2{x, y};
            if (src.x < 0 || src.x >= static_cast<int>(layer.GetWidth())
                || src.y < 0 || src.y >= static_cast<int>(layer.GetHeight()))
            {
                continue;
            }
            region.SetPixel(x, y, layer.GetPixel(src.x, src.y));
        }
    }

    return region;
}

void SpriteRenderer::CopyRect(
    glm::ivec2 pos,
    glm::ivec2 dims,
    Layer source,
    Layer target)
{
    const auto& from = GetLayer(source);
    auto& to = GetLayer(target);

    const auto right  = std::min(pos.x + dims.x, static_cast<int>(to.GetWidth()));
    const auto bottom = std::min(pos.y + dims.y, static_cast<int>(to.GetHeight()));

    for (int y = std::max(pos.y, 0); y < bottom; y++)
    {
        for (int x = std::max(pos.x, 0); x < right; x++)
        {
            to.SetPixel(x, y, from.GetPixel(x, y));
        }
    }
}

Graphics::Texture& SpriteRenderer::GetLayer(Layer layer)
{
    return mLayers[static_cast<unsigned>(layer)];
}

const Graphics::Texture& SpriteRenderer::GetLayer(Layer layer) const
{
    return mLayers[static_cast<unsigned>(layer)];
}

void SpriteRenderer::SetClipRegion(BAK::ClipRegion clipRegion)
{
    mClipRegion = clipRegion;
}

void SpriteRenderer::ClearClipRegion()
{
    mClipRegion.reset();
}

}
