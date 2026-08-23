#pragma once

#include "bak/palette.hpp"
#include "bak/scene/ttmRunner.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/IDialogScene.hpp"

#include "com/logger.hpp"

#include "graphics/sprites.hpp"

#include "gui/core/widget.hpp"
#include "gui/button.hpp"
#include "gui/textBox.hpp"

#include <glm/glm.hpp>

#include <optional>
#include <unordered_map>
#include <vector>

namespace Gui {

class AnimatorStore;
class Backgrounds;
class Font;

class DynamicTTM : public IDialogScene
{
    
public:
    static constexpr double sSecondsPerTick = .017;

    DynamicTTM(
        Graphics::SpriteManager& spriteManager,
        AnimatorStore& animatorStore,
        IGuiManager& guiManager,
        const Font& font,
        const Backgrounds& background,
        std::function<void()>&& sceneFinished,
        std::function<void(unsigned)>&& displayBook);

    Widget* GetScene();

    void BeginScene(std::string adsFile, std::string ttmFile);
    bool AdvanceFrame();

    void DisplayNPCBackground() override {};
    void DisplayPlayerBackground() override {};
    void DialogFinished(const std::optional<BAK::ChoiceIndex>&) override;

private:
    bool RenderDialog(const BAK::ShowDialog&);
    bool StartFade(unsigned startColor, unsigned endColor, unsigned durationIndex, bool fadeIn);
    void ShowNextFrame();
    void FinishFrame(bool scriptFinished);
    void Delay(double seconds);
    void ClearText();
    glm::vec3 GetPaletteColor(unsigned index) const;

    Graphics::SpriteManager& mSpriteManager;
    AnimatorStore& mAnimatorStore;
    IGuiManager& mGuiManager;
    const Font& mFont;
    Widget mSceneFrame;
    Widget mDialogBackground;
    Widget mRenderedElements;
    Widget mFadeRect;

    TextBox mLowerTextBox;
    Button mPopup;
    TextBox mPopupText;

    std::vector<Widget> mSceneElements;
    BAK::TTMRunner mRunner;

    std::optional<BAK::ScriptFrame> mCurrentFrame{};
    unsigned mNextAction{0};

    bool mDelaying = false;
    bool mFading = false;
    bool mWaitForClick = false;
    bool mFramePresented = false;
    bool mSceneComplete = false;
    double mDelay = 0;

    std::unordered_map<unsigned, BAK::Palette> mPaletteSlots;
    unsigned mCurrentPaletteSlot{0};

    Graphics::TextureStore mRenderedFrames;
    Graphics::SpriteManager::TemporarySpriteSheet mRenderedFramesSheet;
    unsigned mCurrentRenderedFrame{0};
    bool mWaitAtNextUpdate{false};
    unsigned mMusicTracksPlayed{0};
    unsigned mDialogType{0};

    std::function<void()> mSceneFinished;
    std::function<void(unsigned)> mDisplayBook;

    const Logging::Logger& mLogger;
};

}
