#include "gui/gdsScene.hpp"

#include "audio/audio.hpp"

#include "bak/bard.hpp"
#include "bak/dialogSources.hpp"
#include "bak/fileBufferFactory.hpp"
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
        BAK::FileBufferFactory::Get().CreateDataBuffer(
            mReference.ToFilename())},
    mSong{mSceneHotspots.mSong},
    mFlavourText{BAK::KeyTarget{0x0}},
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
    mHotspotClicked{},
    mCursor{cursor},
    mGuiManager{guiManager},
    mDialogDisplay{
        glm::vec2{0, 0},
        glm::vec2{320, 240},
        actors,
        backgrounds,
        font,
        gameState},
    mState{State::Idle},
    mPendingGoto{},
    mKickedOut{false},
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

    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(mReference.ToFilename());
    mFlavourText = BAK::KeyTarget{mSceneHotspots.mFlavourText};

    // Needed for repair shops...
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    if (container && container->IsShop())
    {
        auto& shopStats = container->GetShop();
        mGameState.SetShopType_7542(shopStats.mRepairTypes);
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
    mHotspotClicked.reserve(mSceneHotspots.mHotspots.size());
    for (unsigned i = 0; i < mSceneHotspots.mHotspots.size(); i++)
    {
        mHotspotClicked.emplace_back(false);
        auto& hs = mSceneHotspots.mHotspots[i];
        const auto isActive = hs.IsActive(mGameState);
        mLogger.Debug() << "Checked HS: " << hs.mHotspot << " " << isActive << "\n";
        if (isActive)
        {
            mHotspots.emplace_back(
                [this, hs, i](){
                    mLogger.Debug() << "Clicked : " << hs.mHotspot << std::endl;
                    const bool clicked = mHotspotClicked[i];
                    mHotspotClicked[i] = true;
                    HandleHotspotLeftClicked(hs, clicked);
                },
                [this, hs](){
                    HandleHotspotRightClicked(hs);
                },
                cursor,
                mFont,
                hs.mTopLeft,
                hs.mDimensions,
                hs.mHotspot,
                hs.mKeyword - 1 // cursor index
             );
            AddChildBack(&mHotspots.back());
        }
    }

    DisplayNPCBackground();

    if (mFlavourText != BAK::Target{BAK::KeyTarget{0}})
        mDialogDisplay.ShowFlavourText(mFlavourText);
    AddChildBack(&mDialogDisplay);

    mLogger.Debug() << "Constructed @" << std::hex << this << std::dec << "\n";
}

void GDSScene::EnterGDSScene()
{
    unsigned count = 0;
    for (const auto hotspot : mSceneHotspots.mHotspots)
    {
        if (hotspot.IsActive(mGameState) && hotspot.mAction == BAK::HotspotAction::TELEPORT)
        {
            assert(mSceneHotspots.GetTempleNumber());
            mGameState.Apply(BAK::State::SetTempleSeen, *mSceneHotspots.GetTempleNumber());
        }
        if (hotspot.IsActive(mGameState) && hotspot.EvaluateImmediately())
        {
            count++;
            HandleHotspotLeftClicked(hotspot, false);
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

void GDSScene::HandleHotspotLeftClicked(const BAK::Hotspot& hotspot, bool hotspotClicked)
{ 
    mLogger.Debug() << "Hotspot: " << hotspot << "\n"
        << "Tele: " << (mState == State::Teleport) << "\n";

    mGameState.SetItemValue(BAK::Royals{0});

    if (hotspot.mActionArg3 != 0 && hotspot.mAction != BAK::HotspotAction::TEMPLE)
    {
    }

    if (hotspot.mAction == BAK::HotspotAction::DIALOG)
    {
        mGameState.SetDialogContext_7530(hotspotClicked);
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

        mState = State::Dialog;
        auto* container = mGameState.GetContainerForGDSScene(mReference);
        if (container && container->IsShop())
        {
            mGameState.SetBardReward_754d(container->GetShop().mBardingReward);
            mGameState.SetShopType_7542(container->GetShop().mRepairTypes);
        }
        else
        {
            mGameState.SetBardReward_754d(0);

        }
        StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::EXIT)
    {
        mGuiManager.ExitGDSScene();
        return;
    }
    else if (hotspot.mAction == BAK::HotspotAction::TEMPLE)
    {
        auto* container = mGameState.GetContainerForGDSScene(mReference);
        ASSERT(container);
        assert(mSceneHotspots.GetTempleNumber());
        mTemple.EnterTemple(
            BAK::KeyTarget{hotspot.mActionArg3},
            *mSceneHotspots.GetTempleNumber(),
            container->GetShop());
    }
    else if (hotspot.mAction == BAK::HotspotAction::TELEPORT)
    {
        assert(mSceneHotspots.GetTempleNumber());
        if (mGameState.GetChapter() == BAK::Chapter{6}
            && *mSceneHotspots.GetTempleNumber() == BAK::Temple::sChapelOfIshap
            && !mGameState.GetEventStateBool(BAK::GameData::sPantathiansEventFlag))
        {
            mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportBlockedMalacsCrossSource, false, false, this);
        }
        else
        {
            StartDialog(BAK::DialogSources::mTeleportDialogIntro, false);
            mState = State::Teleport;
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
            mState = State::Container;
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
            mState = State::Repair;
        }
        else
        {
            ASSERT(false);
        }
    }
    else if (hotspot.mAction == BAK::HotspotAction::INN)
    {
        mLogger.Debug() << "Inn hotspot: " << hotspot << "\n";
        if (hotspot.mActionArg3 != 0)
        {
            mState = State::Inn;
            StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
        }
        else
        {
            DoInn();
        }
        mLogger.Debug() << "State now: " << static_cast<unsigned>(mState) << "\n";
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
            mState = State::Goto;
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
    mLogger.Debug() << "Dialog finished with choice: " << choice
        << " state: " << static_cast<unsigned>(mState) << "\n";
    if (mState == State::Bard)
    {
        AudioA::AudioManager::Get().PopTrack();
    }

    if (mKickedOut)
    {
        mKickedOut = false;
        mState = State::Idle;
        mGuiManager.ExitGDSScene();
        // Return immediately or the following 
        // actions will take place on destructed GDSScene
        return;
    }
    else if (mState == State::Inn)
    {
        DoInn();
    }
    else if (mState == State::Repair)
    {
        if (!choice || (choice && choice->mValue == BAK::Keywords::sYesIndex))
        {
            auto* container = mGameState.GetContainerForGDSScene(mReference);
            mRepair.EnterRepair(container->GetShop());
        }
    }
    else if (mState == State::Teleport)
    {
        if (mGameState.GetMoreThanOneTempleSeen())
        {
            assert(mSceneHotspots.GetTempleNumber());
            mGuiManager.ShowTeleport(*mSceneHotspots.GetTempleNumber());
        }
        else
        {
            StartDialog(BAK::DialogSources::mTeleportDialogNoDestinations, false);
        }
    }
    else if (mState == State::Container)
    {
        EnterContainer();
    }
    else if (mState == State::Goto)
    {
        if (mGameState.GetEndOfDialogState() != -1)
        {
            mGuiManager.DoFade(.8, [this, pendingGoto=*mPendingGoto]{
                mGuiManager.EnterGDSScene(pendingGoto, []{});
            });
        }
        mPendingGoto.reset();
    }
    else if (mState == State::Dialog)
    {
        mLogger.Debug() << "End of Dialog - " << mSceneHotspots.mUnknown_25 << "\n";
        if (mSceneHotspots.mUnknown_25 != 0)
        {
            if (mGameState.GetBardReward_754d() > 0xfa)
            {
                mGameState.SetBardReward_754d(0xfa);
            }
            auto* container = mGameState.GetContainerForGDSScene(mReference);
            if (container && container->IsShop())
            {
                container->GetShop().mBardingReward = mGameState.GetBardReward_754d();
            }
        }

        if (mGameState.GetEndOfDialogState() == -4)
        {
            mState = State::Idle;
            mGuiManager.ExitGDSScene();
            return;
        }
    }

    mState = State::Idle;

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
    {
        if (container->IsShop())
        {
            mLogger.Debug() << " EnterContainer: " << container->GetShop() << "\n";
        }
        mGuiManager.ShowContainer(container, BAK::EntityType::BAG);
    }
}

void GDSScene::DoInn()
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    assert(container && container->IsShop());
    auto& shopStats = container->GetShop();
    mGuiManager.ShowCamp(true, &container->GetShop());
    mState = State::Idle;
}

void GDSScene::DoBard()
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    assert(container && container->IsShop());
    auto& shopStats = container->GetShop();

    const auto [character, skill] = mGameState.GetPartySkill(
        BAK::SkillType::Barding, true);
    mGameState.SetActiveCharacter(character);

    //if (shopStats.mBardingMaxReward != shopStats.mBardingReward)
    if (shopStats.mBardingReward == 0)
    {
        StartDialog(BAK::DialogSources::mBardingAlreadyDone, false);
    }
    else
    {
        const auto status = BAK::Bard::ClassifyBardAttempt(
            skill, shopStats.mBardingSkill);

        mState = State::Bard;
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
