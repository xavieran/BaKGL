#pragma once

#include "bak/cutscenes.hpp"

#include "gui/bookPlayer.hpp"
#include "gui/core/widget.hpp"
#include "gui/dynamicTTM.hpp"

#include <functional>

namespace Graphics {
class SpriteManager;
}

namespace Gui {
class IGuiManager;
class AnimatorStore;
class Font;
class Backgrounds;

class CutscenePlayer : public Widget
{
public:
    CutscenePlayer(
        Graphics::SpriteManager& spriteManager,
        AnimatorStore& animatorStore,
        const Font& font,
        const Font& bookFont,
        const Backgrounds& background,
        IGuiManager& guiManager,
        std::function<void()>&& cutsceneFinished);

    void QueueAction(BAK::CutsceneAction action);
    void Play();
    bool OnMouseEvent(const MouseEvent& event) override;
    void Advance();
private:
    void BookFinished();
    void SceneFinished();
    void PlayBook(unsigned book);

    bool mTtmPlaying = false;
    std::vector<BAK::CutsceneAction> mActions;

    BookPlayer mBookPlayer;
    DynamicTTM mDynamicTTM;
    IGuiManager& mGuiManager;

    std::function<void()> mCutsceneFinished;
};

}
