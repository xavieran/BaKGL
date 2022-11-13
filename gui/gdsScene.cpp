#include "gui/gdsScene.hpp"

#include "audio/audio.hpp"

#include "bak/bard.hpp"
#include "bak/dialogSources.hpp"
#include "bak/money.hpp"
#include "bak/temple.hpp"

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
    mFont{font},
    mReference{hotspotRef},
    mGameState{gameState},
    mSceneHotspots{
        FileBufferFactory::Get().CreateDataBuffer(
            mReference.ToFilename())},
    mSong{mSceneHotspots.mSong},
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
    mPendingRepair{},
    mPendingContainer{},
    mPendingGoto{},
    mPendingBard{},
    mKickedOut{false},
    mPendingTeleport{},
    mPendingDialog{},
    mTemple{
        mGameState,
        mGuiManager
    },
    mRepair{
        mGameState,
        mGuiManager
    },
    mLogger{Logging::LogState::GetLogger("Gui::GDSScene")}
{
    mLogger.Debug() << "Song: " << mSong << "\n";
    auto textures = Graphics::TextureStore{};
    BAK::TextureFactory::AddScreenToTextureStore(
        textures, "DIALOG.SCX", "OPTIONS.PAL");

    const auto [x, y] = textures.GetTexture(0).GetDims();
    mSpriteManager
        .GetSpriteSheet(mSpriteSheet)
        .LoadTexturesGL(textures);

    SetDimensions(glm::vec2{x, y});

    auto fb = FileBufferFactory::Get().CreateDataBuffer(mReference.ToFilename());
    mFlavourText = BAK::KeyTarget{mSceneHotspots.mFlavourText};

    // Needed for repair shops...
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    if (container && container->IsShop())
    {
        auto& shopStats = container->GetShop();
        mGameState.SetShopType(shopStats.mRepairTypes);
    }

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
        const auto isActive = hs.IsActive(mGameState);
        mLogger.Debug() << "Checked HS: " << hs.mHotspot << " " << isActive << "\n";
        if (isActive)
        {
            mHotspots.emplace_back(
                cursor,
                mFont,
                hs.mTopLeft,
                hs.mDimensions,
                hs.mHotspot,
                hs.mKeyword - 1, // cursor index
                [this, hs](){
                    HandleHotspotLeftClicked(hs);
                },
                [this, hs](){
                    HandleHotspotRightClicked(hs);
                });
            AddChildBack(&mHotspots.back());
        }
    }

    DisplayNPCBackground();

    if (mFlavourText != BAK::Target{BAK::KeyTarget{0x00000}})
        mDialogDisplay.ShowFlavourText(mFlavourText);
    AddChildBack(&mDialogDisplay);

    mLogger.Debug() << "Constructed @" << std::hex << this << std::dec << "\n";
}

void GDSScene::SetTempleSeen()
{
}

void GDSScene::EnterGDSScene()
{
    unsigned count = 0;
    for (const auto hotspot : mSceneHotspots.mHotspots)
    {
        if (hotspot.IsActive(mGameState) && hotspot.mAction == BAK::HotspotAction::TELEPORT)
        {
            mGameState.SetTempleSeen(mSceneHotspots.mTempleIndex);
        }
        if (hotspot.IsActive(mGameState) && hotspot.EvaluateImmediately())
        {
            count++;
            HandleHotspotLeftClicked(hotspot);
        }
    }
    ASSERT(count <= 1);
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
    mLogger.Debug() << "Hotspot: " << hotspot << "\n"
        << "Tele: " << mPendingTeleport << "\n";;

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

        mPendingDialog = true;
        StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::EXIT)
    {
        mGuiManager.ExitGDSScene();
    }
    else if (hotspot.mAction == BAK::HotspotAction::TEMPLE)
    {
        auto* container = mGameState.GetContainerForGDSScene(mReference);
        ASSERT(container);
        mTemple.EnterTemple(
            BAK::KeyTarget{hotspot.mActionArg3},
            mSceneHotspots.mTempleIndex, 
            container->GetShop());
    }
    else if (hotspot.mAction == BAK::HotspotAction::TELEPORT)
    {
        if (mGameState.GetChapter() == BAK::Chapter{6}
            && mSceneHotspots.mTempleIndex == BAK::Temple::sChapelOfIshap
            && !mGameState.GetEventStateBool(BAK::GameData::sPantathiansEventFlag))
        {
            mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportBlockedMalacsCrossSource, false, false, this);
        }
        else
        {
            StartDialog(BAK::DialogSources::mTeleportDialogIntro, false);
            mPendingTeleport = true;
        }
    }
    else if (hotspot.mAction == BAK::HotspotAction::SHOP
        || hotspot.mAction == BAK::HotspotAction::BARMAID
        || hotspot.mAction == BAK::HotspotAction::CONTAINER)
    {
        // FIXME! Need someting a bit more generic than this...
        if (hotspot.mActionArg3 != 0)
        {
            StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
            mPendingContainer = true;
        }
        else
        {
            EnterContainer();
        }
    }
    else if (hotspot.mAction == BAK::HotspotAction::REPAIR)
    {
        if (hotspot.mActionArg3 != 0)
        {
            StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);

            auto* container = mGameState.GetContainerForGDSScene(mReference);
            mLogger.Debug() << container->GetShop() << "\n";
            mPendingRepair = true;
        }
        else
        {
            ASSERT(false);
        }
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
            mGuiManager.DoFade(.8, [this, hotspotRef]{
                mGuiManager.EnterGDSScene(hotspotRef, []{});
            });
        }
    }
}

void GDSScene::HandleHotspotRightClicked(const BAK::Hotspot& hotspot)
{
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";
    StartDialog(hotspot.mTooltip, true);
}

void GDSScene::StartDialog(const BAK::Target target, bool isTooltip)
{
    mDialogDisplay.Clear();
    mGuiManager.StartDialog(target, isTooltip, false, this);
}

void GDSScene::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    if (mPendingBard)
    {
        AudioA::AudioManager::Get().PopTrack();
        mPendingBard = false;
    }

    if (mPendingDialog)
    {
        mPendingDialog = false;
        if (mGameState.GetEndOfDialogState() == -4)
        {
            mGuiManager.ExitGDSScene();
            return;
        }
    }
    else if (mPendingGoto)
    {
        mGuiManager.DoFade(.8, [this, pendingGoto=*mPendingGoto]{
            mGuiManager.EnterGDSScene(pendingGoto, []{});
        });
        mPendingGoto.reset();
    }
    else if (mPendingInn)
    {
        StartDialog(BAK::DialogSources::mInnDialog, false);
        mPendingInn.reset();
    }
    else if (mPendingContainer)
    {
        mPendingContainer = false;
        EnterContainer();
    }
    else if (mPendingRepair)
    {
        mPendingRepair = false;
        if (choice && choice->mValue == BAK::Keywords::sYesIndex)
        {
            auto* container = mGameState.GetContainerForGDSScene(mReference);
            mRepair.EnterRepair(container->GetShop());
        }
    }
    else if (mKickedOut)
    {
        mKickedOut = false;
        mGuiManager.ExitGDSScene();
        // Return immediately or the following 
        // actions will take place on destructed GDSScene
        return;
    }
    else if (mPendingTeleport)
    {
        mPendingTeleport = false;
        
        if (mGameState.GetMoreThanOneTempleSeen())
        {
            mGuiManager.ShowTeleport(mSceneHotspots.mTempleIndex);
        }
        else
        {
            StartDialog(BAK::DialogSources::mTeleportDialogNoDestinations, false);
        }
    }

    mLogger.Debug() << "Dialog finished with choice: " << choice << " , back to flavour text\n";

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
    auto& shopStats = container->GetShop();

    const auto [character, skill] = mGameState.GetParty().GetSkill(
        BAK::SkillType::Barding, true);
    mGameState.SetActiveCharacter(character);

    if (shopStats.mBardingMaxReward != shopStats.mBardingReward)
    {
        StartDialog(BAK::DialogSources::mBardingAlreadyDone, false);
    }
    else
    {
        const auto status = BAK::Bard::ClassifyBardAttempt(
            skill, shopStats.mBardingSkill);

        mPendingBard = true;
        switch (status)
        {
            case BAK::Bard::BardStatus::Failed:
                AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::BAD_BARD);
                break;
            case BAK::Bard::BardStatus::Poor:
                AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::POOR_BARD);
                break;
            case BAK::Bard::BardStatus::Good:
                AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::GOOD_BARD);
                break;
            case BAK::Bard::BardStatus::Best:
                AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::BEST_BARD);
                break;
        }

        const auto reward = BAK::Bard::GetReward(
            status,
            BAK::Sovereigns{shopStats.mBardingMaxReward},
            mGameState.GetChapter());
        BAK::Bard::ReduceAvailableReward(shopStats, reward);
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
            BAK::SkillChange::ExercisedSkill,
            skillMultiplier);

        if (status == BAK::Bard::BardStatus::Failed)
            mKickedOut = true;

        mGameState.SetItemValue(reward);
        StartDialog(GetDialog(status), false);
    }
}

}
