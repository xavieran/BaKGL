#pragma once

#include "bak/image.hpp"
#include "bak/palette.hpp"
#include "bak/scene.hpp"
#include "bak/spriteRenderer.hpp"
#include "bak/ttmRunner.hpp"

#include "com/logger.hpp"

namespace BAK {

class TTMRenderer
{
    
public:
    TTMRenderer(
        std::string adsFile,
        std::string ttmFile);

    Graphics::TextureStore RenderTTM();

private:
    bool AdvanceAction();

    TTMRunner mRunner;

    struct PaletteSlot
    {
        BAK::Palette mPaletteData;
    };
    struct ImageSlot 
    {
        std::vector<BAK::Image> mImages;
    };
    unsigned mCurrentPaletteSlot = 0;
    unsigned mCurrentImageSlot = 0;
    unsigned mImageSaveLayer = 0;
    std::unordered_map<unsigned, BAK::SetClearRegion> mClearRegions;
    std::unordered_map<unsigned, ImageSlot> mImageSlots;
    std::unordered_map<unsigned, PaletteSlot> mPaletteSlots;
    std::unordered_map<unsigned, Graphics::TextureStore> mTextures;

    BAK::SpriteRenderer mRenderer;
    std::optional<BAK::Image> mScreen;

    Graphics::TextureStore mRenderedFrames;

    const Logging::Logger& mLogger;
};

}
