#pragma once

#include "bak/cutscenes.hpp"

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
            ClearChildren();
            AddChildBack(mDynamicTTM.GetScene());
            mDynamicTTM.AdvanceAction();
        }
        else
        {
            Play();
        }
    }

    void SceneFinished()
    {
        mTtmPlaying = false;
        Play();
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

    std::function<void()> mCutsceneFinished;
};

}
