#pragma once

#include "bak/ttmRunner.hpp"

#include "com/logger.hpp"

#include "graphics/sprites.hpp"

#include "gui/core/widget.hpp"
#include "gui/button.hpp"
#include "gui/textBox.hpp"

#include <vector>

namespace Gui {

class AnimatorStore;
class Backgrounds;
class Font;

class DynamicTTM
{
    
public:
    DynamicTTM(
        Graphics::SpriteManager& spriteManager,
        AnimatorStore& animatorStore,
        const Font& font,
        const Backgrounds& background,
        std::function<void()>&& sceneFinished,
        std::function<void(unsigned)>&& displayBook);

    Widget* GetScene();

    void BeginScene(std::string adsFile, std::string ttmFile);
    bool AdvanceAction();

private:
    bool RenderDialog(const BAK::ShowDialog&);
    void ClearText();

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
    unsigned mCurrentRenderedFrame{0};
    bool mWaitAtNextUpdate{false};

    std::function<void()> mSceneFinished;
    std::function<void(unsigned)> mDisplayBook;

    const Logging::Logger& mLogger;
};

}
