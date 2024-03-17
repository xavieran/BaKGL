#include "gui/dynamicTTM.hpp"

#include "bak/imageStore.hpp"
#include "bak/screen.hpp"
#include "bak/textureFactory.hpp"
#include "bak/ttmRenderer.hpp"

#include "bak/dialog.hpp"
#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/types.hpp"

#include "gui/colors.hpp"

namespace Gui {

class CallbackDelay : public IAnimator
{
public:
    CallbackDelay(
        std::function<void()>&& callback,
        double delay)
    :
        mAlive{true},
        mDelay{delay},
        mCallback{std::move(callback)}
    {
    }

    void OnTimeDelta(double delta) override
    {
        mDelay -= delta;
        if (mDelay <= 0)
        {
            mCallback();
            mAlive = false;
        }
    }

    bool IsAlive() const override
    {
        return mAlive;
    }

    bool mAlive;
    double mDelay;
    std::function<void()> mCallback;
};

DynamicTTM::DynamicTTM(
    Graphics::SpriteManager& spriteManager,
    AnimatorStore& animatorStore,
    const Font& font,
    const Backgrounds& backgrounds,
    std::string adsFile,
    std::string ttmFile)
:
    mSpriteManager{spriteManager},
    mAnimatorStore{animatorStore},
    mFont{font},
    mSceneFrame{
        Graphics::DrawMode::Rect,
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        glm::vec4{0},
        glm::vec2{0},
        glm::vec2{1},
        false 
    },
    mDialogBackground{
        Graphics::DrawMode::Sprite,
        backgrounds.GetSpriteSheet(),
        backgrounds.GetScreen("DIALOG.SCX"),
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        glm::vec2{0},
        glm::vec2{320, 200},
        true
    },
    mRenderedElements{
        Graphics::DrawMode::Rect,
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        glm::vec4{0},
        glm::vec2{0},
        glm::vec2{1},
        false 
    },
    mLowerTextBox{
        glm::vec2{15, 125},
        glm::vec2{285, 66}
    },
    mPopup{
        glm::vec2{},
        glm::vec2{},
        Color::buttonBackground,
        Color::buttonHighlight,
        Color::buttonShadow,
        Color::black
    },
    mPopupText{
        glm::vec2{},
        glm::vec2{}
    },
    mSceneElements{},
    mRenderedFramesSheet{mSpriteManager.AddTemporarySpriteSheet()},
    mLogger{Logging::LogState::GetLogger("Gui::DynamicTTM")}
{
    mPopup.AddChildBack(&mPopupText);

    mSceneFrame.AddChildBack(&mDialogBackground);
    mSceneFrame.AddChildBack(&mRenderedElements);
    mSceneElements.reserve(100);
    mLogger.Debug() << "Loading ADS/TTM: " << adsFile << " " << ttmFile << "\n";
    auto adsFb = BAK::FileBufferFactory::Get().CreateDataBuffer(adsFile);
    mSceneSequences = BAK::LoadSceneSequences(adsFb);
    auto ttmFb = BAK::FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    mActions = BAK::LoadDynamicScenes(ttmFb);
    BAK::TTMRenderer renderer(adsFile, ttmFile);
    mRenderedFrames = renderer.RenderTTM();
    mSpriteManager.GetSpriteSheet(mRenderedFramesSheet->mSpriteSheet).LoadTexturesGL(mRenderedFrames);
}

void DynamicTTM::BeginScene()
{

    mLogger.Debug() << "SceneSequences\n";
    for (const auto& [key, sequences] : mSceneSequences)
    {
        mLogger.Debug() << "Key: " << key << "\n";
        for (const auto& sequence : sequences)
        {
            mLogger.Debug() << "  Sequence: " << sequence.mName << "\n";
            for (const auto& scene : sequence.mScenes)
            {
                mLogger.Debug() << "    ADS(" << scene.mInitScene << ", " << scene.mDrawScene << ")\n";
            }
        }
    }

    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mLogger.Debug() << "Next tag: " << nextTag << "\n";
    mCurrentAction = FindActionMatchingTag(nextTag);
    mLogger.Debug() << "Current action: " << mCurrentAction << "\n";
}

void DynamicTTM::AdvanceAction()
{
    if (mDelaying) return;
    mLogger.Debug() << "AdvanceAction" << "\n";
    const auto& action = mActions[mCurrentAction];
    bool nextActionChosen = false;
    bool waitForClick = false;
    mLogger.Debug() << "Handle action: " << action << std::endl;
    std::visit(
        overloaded{
            [&](const BAK::Delay& delay){
                mDelay = static_cast<double>(delay.mDelayMs) / 1000.;
            },
            [&](const BAK::ShowDialog& dialog){
                RenderDialog(dialog);
                if (!dialog.mClearDialog)
                {
                    waitForClick = true;
                }
            },
            [&](const BAK::Update& sr){
                mSceneElements.clear();
                mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mRenderedFramesSheet->mSpriteSheet,
                    Graphics::TextureIndex{mCurrentRenderedFrame++},
                    Graphics::ColorMode::Texture,
                    glm::vec4{1},
                    glm::vec2{0},
                    glm::vec2{320, 200},
                    false 
                );
                mRenderedElements.ClearChildren();
                for (auto& element : mSceneElements)
                {
                    mRenderedElements.AddChildBack(&element);
                }

            },
            [&](const BAK::Purge&){
                AdvanceToNextScene();
                nextActionChosen = true;
            },
            [&](const BAK::GotoTag& sa){
                mCurrentAction = FindActionMatchingTag(sa.mTag);
                nextActionChosen = true;;
            },
            [&](const auto&){}
        },
        action
    );

    if (!nextActionChosen)
    {
        mCurrentAction++;
        return mCurrentAction == mActions.size();
    }
    else
    {
        return mCurrentAction == mActions.size();
    }

    if (!waitForClick)
    {
        mDelaying = true;
        mAnimatorStore.AddAnimator(std::make_unique<CallbackDelay>(
            [&](){
                mDelaying = false;
                AdvanceAction();
            },
            mDelay));
    }

    return false;
}

void DynamicTTM::AdvanceToNextScene()
{
    auto& currentScenes = mSceneSequences[1][mCurrentSequence].mScenes;
    mCurrentSequenceScene++;
    if (mCurrentSequenceScene == currentScenes.size())
    {
        mLogger.Info() << "Finished current scene sequence, moving to next sequence\n";
        mCurrentSequenceScene = 0;
        mCurrentSequence++;
    }

    if (mCurrentSequence == mSceneSequences[1].size())
    {
        mLogger.Info() << "Finished all sequences, current action: "
            << mCurrentAction << " actions size: " << mActions.size() << "\n";
        mCurrentAction = mActions.size();
        mCurrentSequence = 0;
        return;
    }

    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mLogger.Debug() << "Next tag: " << nextTag << "\n";
    mCurrentAction = FindActionMatchingTag(nextTag);
    mLogger.Debug() << "Current action: " << mCurrentAction << "\n";
}

unsigned DynamicTTM::FindActionMatchingTag(unsigned tag)
{
    std::optional<unsigned> foundIndex{};
    for (unsigned i = 0; i < mActions.size(); i++)
    {
        evaluate_if<BAK::SetScene>(mActions[i], [&](const auto& action) {
            if (action.mSceneNumber == tag)
            {
                foundIndex = i;
            }
        });
        if (foundIndex)
        {
            return *foundIndex;
        }
    }

    //throw std::runtime_error("Couldn't find action matching tag: " + std::to_string(tag));
    return 0;
}

void DynamicTTM::RenderDialog(const BAK::ShowDialog& dialog)
{
    if (!dialog.mClearDialog)
    {
        const auto& snippet = BAK::DialogStore::Get().GetSnippet(dialog.mDialogKey);
        auto popup = snippet.GetPopup();
        mLogger.Debug() << "Show snippet;" << snippet << "\n";
        if (popup)
        {
            mPopup.SetPosition(popup->mPos);
            mPopup.SetDimensions(popup->mDims);
            mPopupText.SetPosition(glm::vec2{1});
            mPopupText.SetDimensions(popup->mDims);
            mPopupText.SetText(mFont, snippet.GetText());
            mLowerTextBox.ClearChildren();
            if (!mSceneFrame.HaveChild(&mPopup))
            {
                mSceneFrame.AddChildBack(&mPopup);
            }
        }
        else
        {
            mLowerTextBox.SetText(mFont, snippet.GetText());
            mPopupText.ClearChildren();
            if (!mSceneFrame.HaveChild(&mLowerTextBox))
            {
                mSceneFrame.AddChildBack(&mLowerTextBox);
            }
        }
    }
    else
    {
        mPopupText.ClearChildren();
        mLowerTextBox.ClearChildren();
        if (mSceneFrame.HaveChild(&mPopup))
        {
            mSceneFrame.RemoveChild(&mPopup);
        }
        if (mSceneFrame.HaveChild(&mLowerTextBox))
        {
            mSceneFrame.RemoveChild(&mLowerTextBox);
        }
    }
}

Widget* DynamicTTM::GetScene()
{
    return &mSceneFrame;
}

}
