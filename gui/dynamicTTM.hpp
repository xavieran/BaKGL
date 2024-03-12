#pragma once

#include "bak/scene.hpp"
#include "bak/image.hpp"
#include "bak/spriteRenderer.hpp"
#include "bak/palette.hpp"

#include "com/logger.hpp"

#include "graphics/guiTypes.hpp"
#include "graphics/sprites.hpp"

#include "gui/animatorStore.hpp"
#include "gui/scene.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class DynamicTTM : public NullDialogScene
{
    
public:
    DynamicTTM(
        Graphics::SpriteManager& spriteManager,
        AnimatorStore& animatorStore,
        const Font& font,
        const Backgrounds& background,
        std::string adsFile,
        std::string ttmFile);

    Widget* GetScene();

    void BeginScene();
    void AdvanceAction();

private:
    void AdvanceToNextScene();
    unsigned FindActionMatchingTag(unsigned tag);
    void RenderDialog(const BAK::ShowDialog&);

    Graphics::SpriteManager& mSpriteManager;
    AnimatorStore& mAnimatorStore;
    const Font& mFont;
    Widget mSceneFrame;
    Widget mDialogBackground;
    Widget mRenderedElements;

    TextBox mLowerTextBox;
    Button mPopup;
    TextBox mPopupText;

    std::vector<Widget> mSceneElements;

    std::unordered_map<unsigned, std::vector<BAK::SceneSequence>> mSceneSequences;
    std::vector<BAK::SceneAction> mActions;

    double mDelay = 0;
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
    unsigned mCurrentPaletteSlot;
    unsigned mCurrentImageSlot;
    std::unordered_map<unsigned, ImageSlot> mImageSlots;
    std::unordered_map<unsigned, PaletteSlot> mPaletteSlots;
    std::unordered_map<unsigned, Graphics::TextureStore> mTextures;
    std::unordered_map<unsigned, Graphics::SpriteManager::TemporarySpriteSheet> mImageSprites;

    BAK::SpriteRenderer mRenderer;
    std::optional<BAK::Image> mScreen;
    std::optional<Graphics::Texture> mBackgroundImage;

    const Logging::Logger& mLogger;
};

}
