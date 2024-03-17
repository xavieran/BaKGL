#pragma once

#include "bak/ttmRenderer.hpp"
#include "bak/ttmRunner.hpp"

#include "com/logger.hpp"

#include "graphics/guiTypes.hpp"
#include "graphics/sprites.hpp"

#include "gui/animatorStore.hpp"
#include "gui/scene.hpp"
#include "gui/dialogDisplay.hpp"
#include "gui/core/widget.hpp"

namespace Gui {

class DynamicTTM
{
    
public:
    DynamicTTM(
        Graphics::SpriteManager& spriteManager,
        AnimatorStore& animatorStore,
        const Font& font,
        const Backgrounds& background,
        std::function<void()>&& sceneFinished,
        std::function<void(unsigned)>&& displayBook,
        std::string adsFile,
        std::string ttmFile);

    Widget* GetScene();

    void BeginScene();
    bool AdvanceAction();

private:
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
    BAK::TTMRunner mRunner;

    bool mDelaying = false;
    double mDelay = 0;

    Graphics::TextureStore mRenderedFrames;
    Graphics::SpriteManager::TemporarySpriteSheet mRenderedFramesSheet;
    unsigned mCurrentRenderedFrame = 0;

    std::function<void()> mSceneFinished;
    std::function<void(unsigned)> mDisplayBook;

    const Logging::Logger& mLogger;
};

}
