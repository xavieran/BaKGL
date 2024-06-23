#pragma once

#include "bak/cutscenes.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/bookPlayer.hpp"
#include "gui/core/widget.hpp"
#include "gui/dynamicTTM.hpp"

namespace Gui {

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

    void QueueAction(BAK::CutsceneAction action)
    {
        mActions.emplace_back(action);
    }

    void Play()
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

    bool OnMouseEvent(const MouseEvent& event) override
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

    void Advance()
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

private:
    void BookFinished()
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

    void SceneFinished()
    {
        mTtmPlaying = false;
        mGuiManager.DoFade(1.5, [&]{
            Play();
        });
    }

    void PlayBook(unsigned book)
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

    bool mTtmPlaying = false;
    std::vector<BAK::CutsceneAction> mActions;

    BookPlayer mBookPlayer;
    DynamicTTM mDynamicTTM;
    IGuiManager& mGuiManager;

    std::function<void()> mCutsceneFinished;
};

}
