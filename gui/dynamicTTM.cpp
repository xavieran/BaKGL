#include "gui/dynamicTTM.hpp"

#include "bak/dialogSources.hpp"
#include "bak/imageStore.hpp"
#include "bak/screen.hpp"
#include "bak/textureFactory.hpp"

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
    std::function<void()>&& sceneFinished,
    std::function<void(unsigned)>&& displayBook,
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
    mRunner{adsFile, ttmFile},
    mRenderedFramesSheet{mSpriteManager.AddTemporarySpriteSheet()},
    mSceneFinished{std::move(sceneFinished)},
    mDisplayBook{std::move(displayBook)},
    mLogger{Logging::LogState::GetLogger("Gui::DynamicTTM")}
{
    mPopup.AddChildBack(&mPopupText);

    mSceneFrame.AddChildBack(&mDialogBackground);
    mSceneFrame.AddChildBack(&mRenderedElements);
    mLogger.Debug() << "Loading ADS/TTM: " << adsFile << " " << ttmFile << "\n";
    BAK::TTMRenderer renderer(adsFile, ttmFile);
    mRenderedFrames = renderer.RenderTTM();
    mSpriteManager.GetSpriteSheet(mRenderedFramesSheet->mSpriteSheet).LoadTexturesGL(mRenderedFrames);

    mSceneElements.emplace_back(
        Graphics::DrawMode::Sprite,
        mRenderedFramesSheet->mSpriteSheet,
        Graphics::TextureIndex{0},
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
}

void DynamicTTM::BeginScene()
{
    AdvanceAction();
}

bool DynamicTTM::AdvanceAction()
{
    if (mDelaying) return false;
    auto actionOpt = mRunner.GetNextAction();
    auto action = *actionOpt;

    bool waitForClick = false;
    std::visit(
        overloaded{
            [&](const BAK::Delay& delay){
                mDelay = static_cast<double>(delay.mDelayMs) / 1000.;
            },
            [&](const BAK::ShowDialog& dialog){
                RenderDialog(dialog);
                if (dialog.mDialogType != 0xff)
                {
                    waitForClick = true;
                }
            },
            [&](const BAK::Update& sr){
                mSceneElements.back().SetTexture(
                    Graphics::TextureIndex{mCurrentRenderedFrame++});
            },
            [&](const BAK::Purge&){
                assert(false);
            },
            [&](const BAK::GotoTag& sa){
                assert(false);
            },
            [&](const auto&){}
        },
        action
    );

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

void DynamicTTM::RenderDialog(const BAK::ShowDialog& dialog)
{
    // mDialogType == 5 - display dialog using RunDialog (i.e. No actor names, no default bold)
    // mDialogType == 1 and 4 - similar to above... not sure the difference
    // mDialogType == 3 - same as above - no wait
    // mDialogType == 0 - the usual method
    if (dialog.mDialogType != 0xff && dialog.mDialogKey != 0)
    {
        const auto& snippet = BAK::DialogStore::Get().GetSnippet(
            BAK::DialogSources::GetTTMDialogKey(dialog.mDialogKey));
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
