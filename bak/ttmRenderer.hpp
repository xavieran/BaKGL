#pragma once

#include "bak/scene.hpp"
#include "bak/image.hpp"
#include "bak/spriteRenderer.hpp"
#include "bak/palette.hpp"

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
    void AdvanceToNextScene();
    unsigned FindActionMatchingTag(unsigned tag);

    std::unordered_map<unsigned, std::vector<BAK::SceneSequence>> mSceneSequences;
    std::vector<BAK::SceneAction> mActions;

    unsigned mCurrentAction = 0;
    unsigned mCurrentSequence = 0;
    unsigned mCurrentSequenceScene = 0;

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
