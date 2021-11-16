#include "gui/gdsScene.hpp"

#include "bak/bard.hpp"
#include "bak/dialogSources.hpp"
#include "bak/money.hpp"

#include "com/assert.hpp"

namespace Gui {

GDSScene::GDSScene(
    Cursor& cursor,
    BAK::HotspotRef hotspotRef,
    Graphics::SpriteManager& spriteManager,
    const Actors& actors,
    const Backgrounds& backgrounds,
    const Font& font,
    BAK::GameState& gameState,
    IGuiManager& guiManager)
:
    Widget{
        Graphics::DrawMode::Sprite,
        spriteManager.AddSpriteSheet(),
        Graphics::TextureIndex{0},
        Graphics::ColorMode::Texture,
        glm::vec4{1},
        glm::vec2{0},
        glm::vec2{1},
        false
    },
    mReference{hotspotRef},
    mGameState{gameState},
    mSceneHotspots{
        FileBufferFactory::CreateFileBuffer(
            mReference.ToFilename())},
    mFlavourText{BAK::KeyTarget{0x00000}},
    mSpriteSheet{GetDrawInfo().mSpriteSheet},
    mSpriteManager{spriteManager},
    // bitofa hack - all gds scenes have such a frame
    mFrame{
        Graphics::DrawMode::Rect,
        mSpriteSheet,
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        Color::frameMaroon,
        glm::vec2{14, 10},
        glm::vec2{291, 103},
        false},
    mStaticTTMs{},
    mHotspots{},
    mCursor{cursor},
    mGuiManager{guiManager},
    mDialogDisplay{
        glm::vec2{0, 0},
        glm::vec2{320, 240},
        actors,
        backgrounds,
        font,
        gameState},
    mPendingInn{},
    mPendingGoto{},
    mKickedOut{false},
    mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
{
    auto textures = Graphics::TextureStore{};
    BAK::TextureFactory::AddScreenToTextureStore(
        textures, "DIALOG.SCX", "OPTIONS.PAL");

    const auto [x, y] = textures.GetTexture(0).GetDims();
    mSpriteManager
        .GetSpriteSheet(mSpriteSheet)
        .LoadTexturesGL(textures);

    SetDimensions(glm::vec2{x, y});

    auto fb = FileBufferFactory::CreateFileBuffer(mReference.ToFilename());
    mFlavourText = BAK::KeyTarget{mSceneHotspots.mFlavourText};

    const auto& scene1 = mSceneHotspots.GetScene(
        mSceneHotspots.mSceneIndex1, mGameState);
    const auto& scene2 = mSceneHotspots.GetScene(
        mSceneHotspots.mSceneIndex2, mGameState);

    // Unlikely we ever nest this deep
    mStaticTTMs.reserve(mMaxSceneNesting);
    mStaticTTMs.emplace_back(
        mSpriteManager,
        scene1,
        scene2);

    AddChildBack(&mFrame);
    mFrame.SetInactive();

    // Want our refs to be stable..
    mHotspots.reserve(mSceneHotspots.mHotspots.size());
    for (const auto& hs : mSceneHotspots.mHotspots)
    {
        mHotspots.emplace_back(
            cursor,
            hs.mTopLeft,
            hs.mDimensions,
            hs.mKeyword - 1, // cursor index
            [this, hs](){
                HandleHotspotLeftClicked(hs);
            },
            [this, hs](){
                HandleHotspotRightClicked(hs);
            });

        AddChildBack(&mHotspots.back());
    }

    DisplayNPCBackground();

    if (mFlavourText != BAK::Target{BAK::KeyTarget{0x00000}})
        mDialogDisplay.ShowFlavourText(mFlavourText);
    AddChildBack(&mDialogDisplay);

    mLogger.Debug() << "Constructed @" << std::hex << this << std::dec << "\n";
}

void GDSScene::DisplayNPCBackground()
{
    mFrame.ClearChildren();
    ASSERT(mStaticTTMs.size() > 0);
    mFrame.AddChildBack(mStaticTTMs.back().GetScene());
}

void GDSScene::DisplayPlayerBackground()
{
    mFrame.ClearChildren();
    ASSERT(mStaticTTMs.size() > 0);
    mFrame.AddChildBack(mStaticTTMs.back().GetBackground());
}

void GDSScene::HandleHotspotLeftClicked(const BAK::Hotspot& hotspot)
{ 
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";

    if (hotspot.mAction == BAK::HotspotAction::DIALOG)
    {
        if (hotspot.mActionArg2 != 0x0)
        {
            const auto& scene1 = mSceneHotspots.GetScene(
                mSceneHotspots.mSceneIndex1, mGameState);
            const auto& scene2 = mSceneHotspots.GetScene(
                hotspot.mActionArg2, mGameState);

            // respect the earlier reserve
            ASSERT(mStaticTTMs.size () < mMaxSceneNesting);
            mStaticTTMs.emplace_back(
                mSpriteManager,
                scene1,
                scene2);
            DisplayNPCBackground();
        }

        StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::EXIT)
    {
        mGuiManager.ExitGDSScene();
    }
    else if (hotspot.mAction == BAK::HotspotAction::TEMPLE)
    {
        mGameState.SetDialogContext(mSceneHotspots.mTempleIndex);
        StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::TELEPORT)
    {
        StartDialog(BAK::DialogSources::mTeleportDialogIntro, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::SHOP
        || hotspot.mAction == BAK::HotspotAction::BARMAID
        || hotspot.mAction == BAK::HotspotAction::CONTAINER)
    {
        EnterContainer();
    }
    else if (hotspot.mAction == BAK::HotspotAction::REPAIR)
    {
        StartDialog(BAK::DialogSources::mRepairShopDialog, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::INN)
    {
        if (hotspot.mActionArg3 != 0)
        {
            StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
            mPendingInn = hotspot;
        }
        else
        {
            StartDialog(BAK::DialogSources::mInnDialog, false);
        }
    }
    else if (hotspot.mAction == BAK::HotspotAction::LUTE)
    {
        DoBard();
    }
    else if (hotspot.mAction == BAK::HotspotAction::GOTO)
    {
        const auto dialog = BAK::KeyTarget{hotspot.mActionArg3};
        auto hotspotRef = mReference;
        hotspotRef.mGdsChar = BAK::MakeHotspotChar(hotspot.mActionArg1);
        if (dialog != BAK::KeyTarget{0} && dialog != BAK::KeyTarget{0x10000})
        {
            mPendingGoto = hotspotRef;
            StartDialog(dialog, false);
        }
        else
        {
            mGuiManager.EnterGDSScene(hotspotRef, []{});
        }
    }
}

void GDSScene::HandleHotspotRightClicked(const BAK::Hotspot& hotspot)
{
    // FIXME: There's a crash here if someone left clicks after having right clicked
    // Left click should clear the hotspot active flag...
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";
    StartDialog(hotspot.mTooltip, true);
}

void GDSScene::StartDialog(const BAK::Target target, bool isTooltip)
{
    mDialogDisplay.Clear();
    mGuiManager.StartDialog(target, isTooltip, false, this);
}

void GDSScene::DialogFinished(const std::optional<BAK::ChoiceIndex>&)
{
    if (mPendingGoto)
    {
        mGuiManager.EnterGDSScene(*mPendingGoto, []{});
        mPendingGoto.reset();
    }
    else if (mPendingInn)
    {
        StartDialog(BAK::DialogSources::mInnDialog, false);
        mPendingInn.reset();
    }
    else if (mKickedOut)
    {
        mKickedOut = false;
        mGuiManager.ExitGDSScene();
        // Return immediately or the following 
        // actions will take place on destructed GDSScene
        return;
    }
    mLogger.Debug() << "Dialog finished, back to flavour text\n";

    if (mFlavourText != BAK::Target{BAK::KeyTarget{0x00000}})
        mDialogDisplay.ShowFlavourText(mFlavourText);
    if (mStaticTTMs.size() > 1)
        mStaticTTMs.pop_back();
    DisplayNPCBackground();
}

void GDSScene::EnterContainer()
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    if (container != nullptr)
        mGuiManager.ShowContainer(container);
}

void GDSScene::DoBard()
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    auto& shopStats = container->GetShopData();

    if (shopStats.mBardingMaxReward != shopStats.mBardingReward)
    {
        StartDialog(BAK::DialogSources::mBardingAlreadyDone, false);
    }
    else
    {
        const auto [character, skill] = mGameState.GetParty().GetSkill(
            BAK::SkillType::Barding, true);
        const auto status = BAK::Bard::ClassifyBardAttempt(
            skill, shopStats.mBardingSkill);
        const auto reward = BAK::Bard::GetReward(
            status,
            BAK::Sovereigns{shopStats.mBardingMaxReward},
            mGameState.GetChapter());
        // shop->ReduceAvailableBardReward(reward);
        mGameState.GetParty().GainMoney(reward);
        const auto skillMultiplier = std::invoke([&]{
            if (status == BAK::Bard::BardStatus::Failed
                || status == BAK::Bard::BardStatus::Poor)
                return 1;
            else
                return 2;
            });

        mGameState.GetParty().ImproveSkillForAll(
            BAK::SkillType::Barding,
            3,
            skillMultiplier);

        if (status == BAK::Bard::BardStatus::Failed)
            mKickedOut = true;
        mGameState.SetItemValue(reward);
        mGameState.SetActiveCharacter(character);
        StartDialog(GetDialog(status), false);
    }
}

}
