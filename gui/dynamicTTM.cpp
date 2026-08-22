#include "gui/dynamicTTM.hpp"

#include "audio/audio.hpp"

#include "bak/dialogAction.hpp"
#include "gui/backgrounds.hpp"
#include "gui/colors.hpp"
#include "gui/callbackDelay.hpp"
#include "gui/fontManager.hpp"

#include "bak/dialogSources.hpp"
#include "bak/textureFactory.hpp"
#include "bak/scene/ttmRenderer.hpp"
#include "bak/dialog.hpp"

#include "com/logger.hpp"

#include "graphics/types.hpp"

#include "gui/animator.hpp"
#include "gui/animatorStore.hpp"

#include <array>

namespace Gui {

namespace {

constexpr std::array<double, 7> sFadeDurations = {0, 0.1, 0.4, 0.8, 1.6, 3.2, 6.4};

double FadeDuration(unsigned index)
{
    ASSERT(index < sFadeDurations.size());
    return sFadeDurations[index];
}

// Hack. The fades currently work by "region" because the first 15 palette
// members are used to draw the background and text. The rest of the
// palette colours are used to draw the window area. We choose the fade
// region based on whether the start color is <16 or not. If it is
// then the fade applies to full screen, otherwise to the window.
// If I implemented palette wise rendering at the shader level I could
// do this properly but leaving that for now.
constexpr unsigned sFirstPictureColor = 16;

std::pair<glm::vec2, glm::vec2> FadeRegion(unsigned startColor)
{
    if (startColor < sFirstPictureColor)
    {
        return {glm::vec2{0, 0}, glm::vec2{320, 200}};
    }
    return {glm::vec2{15, 11}, glm::vec2{289, 101}};
}
}

DynamicTTM::DynamicTTM(
    Graphics::SpriteManager& spriteManager,
    AnimatorStore& animatorStore,
    const Font& font,
    const Backgrounds& backgrounds,
    std::function<void()>&& sceneFinished,
    std::function<void(unsigned)>&& displayBook)
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
    mFadeRect{
        RectTag{},
        glm::vec2{0, 0},
        glm::vec2{320, 200},
        glm::vec4{0},
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
    mRunner{},
    mRenderedFramesSheet{},
    mSceneFinished{std::move(sceneFinished)},
    mDisplayBook{std::move(displayBook)},
    mLogger{Logging::LogState::GetLogger("Gui::DynamicTTM")}
{
    mPopup.AddChildBack(&mPopupText);

    mSceneFrame.AddChildBack(&mDialogBackground);
    mSceneFrame.AddChildBack(&mRenderedElements);
}

void DynamicTTM::BeginScene(
    std::string adsFile,
    std::string ttmFile)
{
    mMusicTracksPlayed = 0;
    mLogger.Debug() << "Loading ADS/TTM: " << adsFile << " " << ttmFile << "\n";
    BAK::TTMRenderer renderer(adsFile, ttmFile);
    mRenderedFrames = renderer.RenderTTM();
    mRenderedFramesSheet = mSpriteManager.AddTemporarySpriteSheet();
    mCurrentRenderedFrame = 0;
    mSpriteManager.GetSpriteSheet(mRenderedFramesSheet->mSpriteSheet).LoadTexturesGL(mRenderedFrames);

    mSceneElements.clear();
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

    mDelaying = false;
    mFading = false;
    mWaitForClick = false;
    mFramePresented = false;
    mDelay = 0;
    mCurrentFrame.reset();
    mNextAction = 0;
    mPaletteSlots.clear();
    mCurrentPaletteSlot = 0;
    mFadeRect.SetColor(glm::vec4{0});
    mRunner.LoadTTM(adsFile, ttmFile);

    ClearText();
}

bool DynamicTTM::AdvanceFrame()
{
    if (mDelaying || mFading)
    {
        return false;
    }

    mLogger.Debug() << __FUNCTION__ << " next action: " << mNextAction << " Current frame: " << bool{mCurrentFrame} << "\n";
    if (!mCurrentFrame)
    {
        auto frameOpt = mRunner.GetNextFrame();
        if (!frameOpt)
        {
            mSceneFinished();
            if (mMusicTracksPlayed > 0)
            {
                AudioA::GetAudioManager().PopTrack();
            }
            return true;
        }

        mCurrentFrame = *frameOpt;
        mNextAction = 0;
        mFramePresented = false;
    }

    bool waitForClick = false;
    bool scriptFinished = false;
    while (mNextAction < mCurrentFrame->mActions.size())
    {
        const auto& action = mCurrentFrame->mActions[mNextAction++];
        mLogger.Debug() << "This action: " << action << "\n";
        std::visit(
            overloaded{
                [&](const BAK::Delay& delay){
                    mDelay = static_cast<double>(delay.mTicks) * sSecondsPerTick;
                },
                [&](const BAK::ShowDialog& dialog){
                    waitForClick |= RenderDialog(dialog);
                },
                [&](const BAK::FadeIn& fade){
                    waitForClick |= StartFade(
                        fade.mStartColor, fade.mEndColor, fade.mDurationIndex, true);
                },
                [&](const BAK::FadeOut& fade){
                    waitForClick |= StartFade(
                        fade.mStartColor, fade.mEndColor, fade.mDurationIndex, false);
                },
                [&](const BAK::SlotPalette& sp){
                    mCurrentPaletteSlot = sp.mSlot;
                },
                [&](const BAK::LoadPalette& p){
                    mPaletteSlots.erase(mCurrentPaletteSlot);
                    mPaletteSlots.emplace(mCurrentPaletteSlot, BAK::Palette{p.mPalette});
                },
                [&](const BAK::PlaySoundS& sound){
                    if (sound.mSoundIndex < 255)
                    {
                        AudioA::GetAudioManager().PlaySound(AudioA::SoundIndex{sound.mSoundIndex});
                    }
                    else
                    {
                        mMusicTracksPlayed++;
                        AudioA::GetAudioManager().ChangeMusicTrack(AudioA::MusicIndex{sound.mSoundIndex});
                    }
                },
                [&](const BAK::EndScript&){
                    scriptFinished = true;
                },
                [&](const auto&){}
            },
            action
        );

        if (waitForClick)
        {
            return false;
        }
    }

    mCurrentFrame.reset();

    mWaitForClick = waitForClick;

    FinishFrame(scriptFinished);

    if (scriptFinished)
    {
        AdvanceFrame();
    }

    return false;
}

glm::vec3 DynamicTTM::GetPaletteColor(unsigned index) const
{
    if (!mPaletteSlots.contains(mCurrentPaletteSlot))
    {
        return glm::vec3{0};
    }

    return glm::vec3{mPaletteSlots.at(mCurrentPaletteSlot).GetColor(index)};
}

bool DynamicTTM::StartFade(unsigned startColor, unsigned endColor, unsigned durationIndex, bool fadeIn)
{
    if (mSceneFrame.HaveChild(&mFadeRect))
    {
        mSceneFrame.RemoveChild(&mFadeRect);
    }
    mSceneFrame.AddChildBack(&mFadeRect);

    const auto color = GetPaletteColor(endColor);
    const auto begin = glm::vec4{color, fadeIn ? 1.0f : 0.0f};
    const auto end = glm::vec4{color, fadeIn ? 0.0f : 1.0f};

    const auto [pos, dims] = FadeRegion(startColor);
    mFadeRect.SetPosition(pos);
    mFadeRect.SetDimensions(dims);

    if (fadeIn)
    {
        ShowNextFrame();
        mFramePresented = true;
    }

    const auto duration = FadeDuration(durationIndex);
    if (duration == 0)
    {
        mFadeRect.SetColor(end);
        return false;
    }

    mFadeRect.SetColor(begin);
    mFading = true;
    mAnimatorStore.AddAnimator(std::make_unique<LinearAnimator>(
        duration,
        begin,
        end,
        [this](const auto& delta){
            mFadeRect.SetColor(mFadeRect.GetDrawInfo().mColor + delta);
            return false;
        },
        [this, end](){
            mFadeRect.SetColor(end);
            mFading = false;
            AdvanceFrame();
        }));
    return true;
}

void DynamicTTM::FinishFrame(bool scriptFinished)
{
    if (!mFramePresented)
    {
        ShowNextFrame();
        mFramePresented = true;
    }

    if (!scriptFinished)
    {
        mFadeRect.SetColor(glm::vec4{0});
    }

    if (!mWaitForClick)
    {
        ClearText();
        Delay(mDelay);
    }
}

void DynamicTTM::ShowNextFrame()
{
    if (mCurrentRenderedFrame < mRenderedFrames.GetTextures().size())
    {
        mSceneElements.back().SetTexture(
            Graphics::TextureIndex{mCurrentRenderedFrame++});
    }
}

void DynamicTTM::Delay(double seconds)
{
    mDelaying = true;
    mAnimatorStore.AddAnimator(std::make_unique<CallbackDelay>(
        [&](){
            mDelaying = false;
            AdvanceFrame();
        },
        seconds));
}

bool DynamicTTM::RenderDialog(const BAK::ShowDialog& dialog)
{
    // mDialogType == 5 - display dialog using RunDialog (i.e. No actor names, no default bold)
    // mDialogType == 1 and 4 - similar to above... not sure the difference
    // mDialogType == 3 - same as above - no wait
    // mDialogType == 0 - the usual method
    if (dialog.mDialogType == 2)
    {
        mDisplayBook(dialog.mDialogKey.value_or(0));
        return true;
    }

    if (dialog.mDialogType != 0xff && dialog.mDialogKey
        && *dialog.mDialogKey != 0 && *dialog.mDialogKey != 0xff)
    {
        const auto& snippet = BAK::DialogStore::Get().GetSnippet(
            BAK::DialogSources::GetTTMDialogKey(*dialog.mDialogKey));
        auto popup = snippet.GetPopup();
        mLogger.Debug() << "Show snippet;" << snippet << "\n";
        if (popup)
        {
            mPopup.SetPosition(popup->mPos);
            mPopup.SetDimensions(popup->mDims);
            mPopupText.SetPosition(glm::vec2{1});
            mPopupText.SetDimensions(popup->mDims);
            mPopupText.SetText(mFont, snippet.GetText(), true, true);
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

        return true;
    }
    else
    {
        ClearText();
    }
    return false;
}

void DynamicTTM::ClearText()
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

Widget* DynamicTTM::GetScene()
{
    return &mSceneFrame;
}

}
