#include "gui/dynamicTTM.hpp"

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
            [&](const BAK::SlotPalette& sp){
                mCurrentPaletteSlot = sp.mSlot;
            },
            [&](const BAK::LoadPalette& p){
                mPaletteSlots.erase(mCurrentPaletteSlot);
                mPaletteSlots.emplace(mCurrentPaletteSlot, BAK::Palette{p.mPalette});
            },
            [&](const BAK::SlotImage& sp){
                mCurrentImageSlot = sp.mSlot;
            },
            [&](const BAK::Delay& delay){
                mDelay = static_cast<double>(delay.mDelayMs) / 1000.;
            },
            [&](const BAK::LoadImage& p){
                auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(p.mImage);
                mImageSlots.erase(mCurrentImageSlot);
                mImageSlots.emplace(mCurrentImageSlot, BAK::LoadImages(fb));
                mLogger.Debug() << "Loaded image: " << p.mImage << " to slot: " << mCurrentImageSlot
                    << " has " << mImageSlots.at(mCurrentImageSlot).mImages.size() << " images\n";
            },
            [&](const BAK::LoadScreen& p){
                auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(p.mScreenName);
                mScreen = BAK::LoadScreenResource(fb);
            },
            [&](const BAK::DrawScreen& sa){
                if (sa.mArg1 == 3 || sa.mArg2 == 3)
                {
                    mRenderer.GetSavedImagesLayer0() = {320, 200};
                    mRenderer.GetSavedImagesLayer1() = {320, 200};
                    mRenderer.GetSavedImagesLayerBG() = {320, 200};
                }
                if (mScreen && mPaletteSlots.contains(mCurrentPaletteSlot))
                {
                    mRenderer.RenderSprite(
                        *mScreen,
                        mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                        glm::ivec2{0, 0},
                        false,
                        mRenderer.GetForegroundLayer());
                }
            },
            [&](const BAK::DrawSprite& sa){
                const auto imageSlot = sa.mImageSlot;
                assert(mImageSlots.contains(sa.mImageSlot));
                assert(static_cast<unsigned>(sa.mSpriteIndex) 
                        < mImageSlots.at(sa.mImageSlot).mImages.size());

                mRenderer.RenderSprite(
                    mImageSlots.at(sa.mImageSlot).mImages[sa.mSpriteIndex],
                    mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                    glm::ivec2{sa.mX, sa.mY},
                    sa.mFlippedInY,
                    mRenderer.GetForegroundLayer());
            },
            [&](const BAK::ShowDialog& dialog){
                RenderDialog(dialog);
                if (!dialog.mClearDialog)
                {
                    waitForClick = true;
                }
            },
            [&](const BAK::Update& sr){
                auto textures = Graphics::TextureStore{};
                if (mScreen)
                {
                    mRenderer.RenderSprite(
                        *mScreen,
                        mPaletteSlots.at(mCurrentPaletteSlot).mPaletteData,
                        glm::ivec2{0, 0}, false, mRenderer.GetBackgroundLayer());
                }
                if (mBackgroundImage)
                {
                    const auto& texture = *mBackgroundImage;
                    mRenderer.RenderTexture(
                        texture,
                        glm::ivec2{0, 0},
                        mRenderer.GetBackgroundLayer());
                }

                mRenderer.RenderTexture(
                    mRenderer.GetSavedImagesLayerBG(),
                    glm::ivec2{0},
                    mRenderer.GetBackgroundLayer());

                mRenderer.RenderTexture(
                    mRenderer.GetSavedImagesLayer0(),
                    glm::ivec2{0},
                    mRenderer.GetBackgroundLayer());

                mRenderer.RenderTexture(
                    mRenderer.GetSavedImagesLayer1(),
                    glm::ivec2{0},
                    mRenderer.GetBackgroundLayer());

                mRenderer.RenderTexture(
                    mRenderer.GetForegroundLayer(),
                    glm::ivec2{0, 0},
                    mRenderer.GetBackgroundLayer());

                auto bg = mRenderer.GetBackgroundLayer();
                bg.Invert();
                textures.AddTexture(bg);

                auto temporarySpriteSheet = mSpriteManager.AddTemporarySpriteSheet();
                mImageSprites[0] = std::move(temporarySpriteSheet);
                mSpriteManager.GetSpriteSheet(mImageSprites[0]->mSpriteSheet).LoadTexturesGL(textures);

                mSceneElements.clear();
                mSceneElements.emplace_back(
                    Graphics::DrawMode::Sprite,
                    mImageSprites[0]->mSpriteSheet,
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
                mRenderer.GetForegroundLayer() = Graphics::Texture{320, 200};
                mRenderer.GetBackgroundLayer() = Graphics::Texture{320, 200};

            },
            [&](const BAK::SaveImage& si){
                mRenderer.SaveImage(si.pos, si.dims, mImageSaveLayer);
            },
            [&](const BAK::SetClearRegion& si){
                mClearRegions.emplace(mImageSaveLayer, si);
            },
            [&](const BAK::ClearSaveLayer& si){
                const auto& clearRegion = mClearRegions.at(mImageSaveLayer);
                mRenderer.ClearSaveLayer(clearRegion.pos, clearRegion.dims, si.mLayer);
            },
            [&](const BAK::SaveBackground&){
                mRenderer.GetSavedImagesLayer0() = {320, 200};
                mRenderer.GetSavedImagesLayer1() = {320, 200};
                mRenderer.SaveImage({0, 0}, {320, 200}, 2);
            },
            [&](const BAK::DrawRect& sr){
            },
            [&](const BAK::ClipRegion& a){
                mRenderer.SetClipRegion(a);
            },
            [&](const BAK::DisableClipRegion&){
                mRenderer.ClearClipRegion();
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
        if (mCurrentAction == mActions.size()) mCurrentAction = 0;
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
        mCurrentSequence = 0;
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
