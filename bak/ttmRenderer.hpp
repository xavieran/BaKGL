#pragma once

#include "bak/image.hpp"
#include "bak/spriteRenderer.hpp"
#include "bak/ttmRunner.hpp"

#include "com/logger.hpp"

#include <unordered_map>

namespace BAK {

class Palette;
class TTMRunner;

class TTMRenderer
{
    
public:
    TTMRenderer(
        std::string adsFile,
        std::string ttmFile);

    Graphics::TextureStore RenderTTM();

private:
    bool AdvanceAction();

    void RenderFrame();

    TTMRunner mRunner;

    struct PaletteSlot
    {
        Palette mPaletteData;
    };
    struct ImageSlot 
    {
        std::vector<Image> mImages;
    };

    unsigned mCurrentPaletteSlot = 0;
    unsigned mCurrentImageSlot = 0;
    unsigned mImageSaveLayer = 0;
    std::unordered_map<unsigned, BAK::SaveRegionToLayer> mClearRegions;
    std::unordered_map<unsigned, Graphics::Texture> mSaves;
    std::unordered_map<unsigned, ImageSlot> mImageSlots;
    std::unordered_map<unsigned, PaletteSlot> mPaletteSlots;
    std::unordered_map<unsigned, Graphics::TextureStore> mTextures;

    SpriteRenderer mRenderer;
    std::optional<BAK::Image> mScreen;

    Graphics::TextureStore mRenderedFrames;

    const Logging::Logger& mLogger;
};

}
