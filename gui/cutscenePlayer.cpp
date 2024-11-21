#include "gui/cutscenePlayer.hpp"

#include "bak/cutscenes.hpp"

#include "gui/animatorStore.hpp"
#include "gui/backgrounds.hpp"
#include "gui/IGuiManager.hpp"
#include "gui/bookPlayer.hpp"
#include "gui/core/widget.hpp"
#include "gui/dynamicTTM.hpp"

namespace Gui {

CutscenePlayer::CutscenePlayer(
    Graphics::SpriteManager& spriteManager,
    AnimatorStore& animatorStore,
    const Font& font,
    const Font& bookFont,
    const Backgrounds& background,
    IGuiManager& guiManager,
    std::function<void()>&& cutsceneFinished)
:
    Widget(RectTag{}, glm::vec2{}, glm::vec2{320, 200}, glm::vec4{}, true),
    mBookPlayer(
        spriteManager,
        bookFont,
        background,
        [&](){ BookFinished(); }),
    mDynamicTTM(
        spriteManager,
        animatorStore,
        font,
        background,
        [&](){ SceneFinished(); },
        [&](auto book){ PlayBook(book); }),
    mGuiManager{guiManager},
    mCutsceneFinished{std::move(cutsceneFinished)}
{
}

void CutscenePlayer::QueueAction(BAK::CutsceneAction action)
{
    mActions.emplace_back(action);
}

void CutscenePlayer::Play()
{
    if (mActions.empty())
    {
        mCutsceneFinished();
        return;
    }

    auto action = *mActions.begin();
    mActions.erase(mActions.begin());
    std::visit(overloaded{
        [&](const BAK::TTMScene& scene)
        {
            mDynamicTTM.BeginScene(scene.mAdsFile, scene.mTTMFile);
            ClearChildren();
            AddChildBack(mDynamicTTM.GetScene());
            mTtmPlaying = true;
            mDynamicTTM.AdvanceAction();
        },
        [&](const BAK::BookChapter& book)
        {
            mBookPlayer.PlayBook(book.mBookFile);
            ClearChildren();
            AddChildBack(mBookPlayer.GetBackground());
        }},
        action);
}

bool CutscenePlayer::OnMouseEvent(const MouseEvent& event)
{
    const auto result = std::visit(overloaded{
        [this](const LeftMousePress& p){ Advance(); return true; },
        [](const auto& p){ return false; }
        },
        event);

    if (result)
        return result;

    return false;
}

void CutscenePlayer::Advance()
{
    if (GetChildren()[0] == mDynamicTTM.GetScene())
    {
        mDynamicTTM.AdvanceAction();
    }
    else if (GetChildren()[0] == mBookPlayer.GetBackground())
    {
        mBookPlayer.AdvancePage();
    }
}

void CutscenePlayer::BookFinished()
{
    if (mTtmPlaying)
    {
        mGuiManager.DoFade(1.5, [&]{
            ClearChildren();
            AddChildBack(mDynamicTTM.GetScene());
            mDynamicTTM.AdvanceAction();
        });
    }
    else
    {
        mGuiManager.DoFade(1.5, [&]{ Play(); });
    }
}

void CutscenePlayer::SceneFinished()
{
    mTtmPlaying = false;
    mGuiManager.DoFade(1.5, [&]{
        Play();
    });
}

void CutscenePlayer::PlayBook(unsigned book)
{
    ClearChildren();
    AddChildBack(mBookPlayer.GetBackground());
    std::stringstream ss{};
    ss << "C";
    ss << (book / 10) % 10;
    ss << book % 10;
    ss << ".BOK";
    mBookPlayer.PlayBook(ss.str());
}

}
