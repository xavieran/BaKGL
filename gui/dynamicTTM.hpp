#pragma once

#include "bak/scene.hpp"
#include "bak/image.hpp"
#include "bak/spriteRenderer.hpp"
#include "bak/palette.hpp"

#include "com/logger.hpp"

#include "graphics/guiTypes.hpp"
#include "graphics/sprites.hpp"

#include "gui/scene.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class DynamicTTM 
{
public:
    DynamicTTM(
        Graphics::SpriteManager& spriteManager,
        std::string adsFile,
        std::string ttmFile);

    Widget* GetScene();

    void BeginScene();
    void AdvanceAction();

private:
    Graphics::SpriteManager& mSpriteManager;
    Widget mSceneFrame;
    std::optional<Widget> mDialogBackground;

    std::vector<Widget> mSceneElements;

    std::unordered_map<unsigned, std::vector<BAK::SceneSequence>> mSceneSequences;
    std::map<unsigned, BAK::DynamicScene> mScenes;

    BAK::DynamicScene* mCurrentScene;
    unsigned mCurrentAction = 0;
    unsigned mCurrentSequence = 0;
    unsigned mCurrentSequenceScene = 0;
    bool mPlayAllScenes = false;
    std::vector<unsigned> mAllSceneKeys;

    struct PaletteSlot
    {
        BAK::Palette mPaletteData;
    };
    struct ImageSlot 
    {
        std::vector<BAK::Image> mImages;
    };
    unsigned mCurrentPaletteSlot;
    unsigned mCurrentImageSlot;
    std::unordered_map<unsigned, ImageSlot> mImageSlots;
    std::unordered_map<unsigned, PaletteSlot> mPaletteSlots;
    std::unordered_map<unsigned, Graphics::TextureStore> mTextures;
    std::unordered_map<unsigned, Graphics::SpriteManager::TemporarySpriteSheet> mImageSprites;

    BAK::SpriteRenderer mRenderer;
    std::optional<std::pair<Graphics::Texture, glm::ivec2>> mSavedImage;
    std::optional<BAK::Image> mScreen;
    std::optional<Graphics::Texture> mBackgroundImage;

    const Logging::Logger& mLogger;
};

}
