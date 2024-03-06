#include "bak/image.hpp"
#include "bak/palette.hpp"

#include "graphics/texture.hpp"


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

    void RenderSprite(
        BAK::Image sprite,
        BAK::Palette palette,
        glm::ivec2 dims)
    {
        for (unsigned x = 0; x < sprite.GetWidth(); x++)
        {
            for (unsigned y = 0; y < sprite.GetHeight(); y++)
            {
                mForegroundLayer.SetPixel(
                    dims.x + x,
                    dims.y + y,
                    palette.GetColor(sprite.GetPixel(x, y)));
            }
        }
    }

    private:
    Graphics::Texture mForegroundLayer;
    Graphics::Texture mBackgroundLayer;
};

}
