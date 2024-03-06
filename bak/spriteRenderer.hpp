#include "bak/image.hpp"
#include "bak/palette.hpp"

#include "bak/sceneData.hpp"

#include "graphics/texture.hpp"

#include <optional>

namespace BAK {

// Used to render TTM scenes
class SpriteRenderer
{
public:
    SpriteRenderer()
    :
        mForegroundLayer{320, 200},
        mBackgroundLayer{320, 200}
    {
    }

    void RenderTexture(
        const Graphics::Texture& texture,
        glm::ivec2 pos,
        bool background = false)
    {
        for (int x = 0; x < static_cast<int>(texture.GetWidth()); x++)
        {
            for (int y = 0; y < static_cast<int>(texture.GetHeight()); y++)
            {
                const auto pixelPos = pos + glm::ivec2{x, y};
                if (mClipRegion)
                {
                    if (pixelPos.x < mClipRegion->mTopLeft.x || pixelPos.x > mClipRegion->mBottomRight.x
                        || pixelPos.y < mClipRegion->mTopLeft.y || pixelPos.y > mClipRegion->mBottomRight.y)
                    {
                        continue;
                    }
                }
                const auto color = texture.GetPixel(x, y);
                if (std::abs(0.0 - color.a) < .0001) continue;
                if (pixelPos.x > 320 || pixelPos.y > 200) continue;

                auto& layer = background ? mBackgroundLayer : mForegroundLayer;
                layer.SetPixel(
                    pixelPos.x,
                    pixelPos.y,
                    color);
            }
        }
    }

    void RenderSprite(
        BAK::Image sprite,
        BAK::Palette palette,
        glm::ivec2 pos,
        bool flipped,
        bool background=false)
    {
        for (int x = 0; x < static_cast<int>(sprite.GetWidth()); x++)
        {
            for (int y = 0; y < static_cast<int>(sprite.GetHeight()); y++)
            {
                const auto pixelPos = pos + glm::ivec2{
                    flipped ? sprite.GetWidth() - x : x,
                    y};
                if (mClipRegion)
                {
                    if (pixelPos.x < mClipRegion->mTopLeft.x || pixelPos.x > mClipRegion->mBottomRight.x
                        || pixelPos.y < mClipRegion->mTopLeft.y || pixelPos.y > mClipRegion->mBottomRight.y)
                    {
                        continue;
                    }
                }
                else if (pixelPos.x < 0 || pixelPos.x > 320
                        || pixelPos.y < 0 || pixelPos.y > 200)
                {
                    continue;
                }
                const auto color = palette.GetColor(sprite.GetPixel(x, y));
                if (color.a == 0) continue;
                if (pixelPos.x > 320
                    || pixelPos.y > 200) continue;

                auto& layer = background ? mBackgroundLayer : mForegroundLayer;
                layer.SetPixel(
                    pixelPos.x,
                    pixelPos.y,
                    color);
            }
        }
    }

    void Clear()
    {
        mForegroundLayer = Graphics::Texture{320, 200};
        mBackgroundLayer = Graphics::Texture{320, 200};
    }

    Graphics::Texture GetForegroundLayer() const
    {
        return mForegroundLayer;
    }

    Graphics::Texture GetBackgroundLayer() const
    {
        return mBackgroundLayer;
    }

    void SetClipRegion(BAK::ClipRegion clipRegion)
    {
        mClipRegion = clipRegion;
    }

    void ClearClipRegion()
    {
        mClipRegion.reset();
    }

    Graphics::Texture SaveImage(glm::ivec2 pos, glm::ivec2 dims)
    {
        auto image = Graphics::Texture{static_cast<unsigned>(dims.x), static_cast<unsigned>(dims.y)};
        for (int x = 0; x < dims.x; x++)
        {
            for (int y = 0; y < dims.y; y++)
            {
                image.SetPixel(x, y, mForegroundLayer.GetPixel(x + pos.x, y + pos.y));
            }
        }
        return image;
    }
private:
    Graphics::Texture mForegroundLayer;
    Graphics::Texture mBackgroundLayer;
    std::optional<BAK::ClipRegion> mClipRegion;
};

}
