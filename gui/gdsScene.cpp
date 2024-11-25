#include "gui/gdsScene.hpp"

#include "audio/audio.hpp"

#include "bak/bard.hpp"
#include "bak/dialogSources.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/gameState.hpp"
#include "bak/textureFactory.hpp"
#include "bak/temple.hpp"
#include "bak/state/temple.hpp"

#include "com/assert.hpp"

#include "gui/IGuiManager.hpp"
#include "gui/cursor.hpp"

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
    mSpriteSheet{spriteManager.AddTemporarySpriteSheet()},
    mFont{font},
    mReference{hotspotRef},
    mGameState{gameState},
    mSceneHotspots{
        BAK::FileBufferFactory::Get().CreateDataBuffer(
            mReference.ToFilename())},
    mFlavourText{BAK::KeyTarget{0x0}},
    mSpriteManager{spriteManager},
    // bitofa hack - all gds scenes have such a frame
    mFrame{
        Graphics::DrawMode::Rect,
        mSpriteSheet->mSpriteSheet,
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
    mPendingAction{},
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
    SetSpriteSheet(mSpriteSheet->mSpriteSheet);
    auto textures = Graphics::TextureStore{};
    BAK::TextureFactory::AddScreenToTextureStore(
        textures, "DIALOG.SCX", "OPTIONS.PAL");

    const auto dims = textures.GetTexture(0).GetDims();
    mSpriteManager
        .GetSpriteSheet(mSpriteSheet->mSpriteSheet)
        .LoadTexturesGL(textures);

    SetDimensions(dims);

    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(mReference.ToFilename());
    mFlavourText = BAK::KeyTarget{mSceneHotspots.mFlavourText};
    if (mFlavourText == BAK::KeyTarget{0x10000}) mFlavourText = BAK::KeyTarget{0};

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

    if (mFlavourText != BAK::KeyTarget{0})
        mDialogDisplay.ShowFlavourText(mFlavourText);
    AddChildBack(&mDialogDisplay);

    mLogger.Debug() << "Constructed @" << std::hex << this << std::dec << "\n";
}

void GDSScene::EnterGDSScene()
{
    std::optional<BAK::Hotspot> evaluateImmediately{};
    for (const auto hotspot : mSceneHotspots.mHotspots)
    {
        if (hotspot.IsActive(mGameState) && hotspot.mAction == BAK::HotspotAction::TELEPORT)
        {
            assert(mSceneHotspots.GetTempleNumber());
            mGameState.Apply(BAK::State::SetTempleSeen, *mSceneHotspots.GetTempleNumber());
        }
        if (hotspot.IsActive(mGameState) && hotspot.EvaluateImmediately())
        {
            assert(!evaluateImmediately);
            evaluateImmediately = hotspot;
        }
    }

    mPendingAction.reset();
    mPendingGoto.reset();

    if (evaluateImmediately)
    {
        HandleHotspotLeftClicked(*evaluateImmediately, false);
    }
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
    mLogger.Debug() << "Hotspot: " << hotspot << "\n";

    mGameState.SetItemValue(BAK::Royals{0});

    mPendingAction = hotspot.mAction;
    if (hotspot.mAction == BAK::HotspotAction::GOTO)
    {
        auto hotspotRef = mReference;
        hotspotRef.mGdsChar = BAK::MakeHotspotChar(hotspot.mActionArg1);
        mPendingGoto = hotspotRef;
    }

    if ((hotspot.mActionArg3 != 0 && hotspot.mActionArg3 != 0x10000)
        && hotspot.mAction != BAK::HotspotAction::TEMPLE)
    {
        if (hotspot.mActionArg2 != 0)
        {
            const auto& scene1 = mSceneHotspots.GetScene(
                mSceneHotspots.mSceneIndex1, mGameState);
            const auto& scene2 = mSceneHotspots.GetScene(
                hotspot.mActionArg2, mGameState);
            AddStaticTTM(scene1, scene2);
        }

        mGameState.SetDialogContext_7530(hotspotClicked);
        mGameState.SetBardReward_754d(0);
        
        auto* container = mGameState.GetContainerForGDSScene(mReference);
        if (container && container->IsShop())
        {
            mGameState.SetBardReward_754d(container->GetShop().mBardingReward);
            mGameState.SetShopType_7542(container->GetShop().mRepairTypes);
        }

        StartDialog(BAK::KeyTarget{hotspot.mActionArg3}, false);
    }
    else if (hotspot.mAction == BAK::HotspotAction::TEMPLE)
    {
        DoTemple(BAK::KeyTarget{hotspot.mActionArg3});
        mPendingAction.reset();
    }
    else if (hotspot.mAction == BAK::HotspotAction::TELEPORT)
    {
        assert(mSceneHotspots.GetTempleNumber());
        if (mGameState.GetChapter() == BAK::Chapter{6}
            && *mSceneHotspots.GetTempleNumber() == BAK::Temple::sChapelOfIshap
            && !mGameState.ReadEventBool(BAK::GameData::sPantathiansEventFlag))
        {
            mGuiManager.StartDialog(BAK::DialogSources::mTeleportDialogTeleportBlockedMalacsCrossSource, false, false, this);
            mPendingAction.reset();
        }
        else
        {
            StartDialog(BAK::DialogSources::mTeleportDialogIntro, false);
        }
    }
    else
    {
        EvaluateHotspotAction();
    }
}


void GDSScene::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    if (mKickedOut)
    {
        mPendingAction = BAK::HotspotAction::EXIT;
        mKickedOut = false;
    }

    if (mBarding)
    {
        AudioA::GetAudioManager().PopTrack();
        mBarding = false;
    }

    if (mFlavourText != BAK::KeyTarget{0})
        mDialogDisplay.ShowFlavourText(mFlavourText);

    if (mStaticTTMs.size() > 1)
    {
        for (unsigned i = 1; i < mStaticTTMs.size(); i++)
        {
            mStaticTTMs.pop_back();
        }
    }

    DisplayNPCBackground();

    if (!mPendingAction)
    {
        mLogger.Debug() << "No pending action at end of dialog, not evaluating further\n";
        return;
    }

    const auto index = mGameState.GetEndOfDialogState() + 5;
    // After dialog finished...
    switch (index)
    {
    case 4: mPendingAction = BAK::HotspotAction::UNKNOWN_0; break;
    case 3: mPendingAction = BAK::HotspotAction::BARMAID; break;
    case 2: mPendingAction = BAK::HotspotAction::INN; break;
    case 1: mPendingAction = BAK::HotspotAction::EXIT; break;
    case 0: mPendingAction = BAK::HotspotAction::REPAIR_2; break;
    default:
        break;
    }
    
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    if (container && container->IsShop())
    {
        if (mGameState.GetBardReward_754d() > 0xfa)
        {
            mGameState.SetBardReward_754d(0xfa);
        }
        container->GetShop().mBardingReward = mGameState.GetBardReward_754d();
    }
    mGameState.SetBardReward_754d(0);

    EvaluateHotspotAction();
}

void GDSScene::AddStaticTTM(BAK::Scene scene1, BAK::Scene scene2)
{
    ASSERT(mStaticTTMs.size () < mMaxSceneNesting);
    mLogger.Debug() << __FUNCTION__ << " " << scene1 << " --- " << scene2 << " \n";
    mStaticTTMs.emplace_back(
        mSpriteManager,
        scene1,
        scene2);
    DisplayNPCBackground();
}

void GDSScene::EvaluateHotspotAction()
{
    if (!mPendingAction)
    {
        mLogger.Debug() << "No action present, not evaluating\n";
        return;
    }

    if (*mPendingAction == BAK::HotspotAction::SHOP)
    {
        EnterContainer();
    }
    else if (*mPendingAction == BAK::HotspotAction::CONTAINER)
    {
        EnterContainer();
    }
    else if (*mPendingAction == BAK::HotspotAction::LUTE)
    {
        DoBard();
    }
    else if (*mPendingAction == BAK::HotspotAction::BARMAID)
    {
        EnterContainer();
    }
    else if (*mPendingAction == BAK::HotspotAction::INN)
    {
        if (mGameState.GetChapter() == BAK::Chapter{5})
        {
            auto* container = mGameState.GetContainerForGDSScene(mReference);
            assert(container && container->IsShop());
            // 0xdb1c Eortis/rusalka qeust?
            if (!mGameState.ReadEventBool(0xdb1c))
            {
                container->GetShop().mInnCost = 0x48;
            }
            else
            {
                container->GetShop().mInnCost = 0xa;
            }
        }
        DoInn();
    }
    else if (*mPendingAction == BAK::HotspotAction::REPAIR_2
        || *mPendingAction == BAK::HotspotAction::REPAIR)
    {
        DoRepair();
    }
    else if (*mPendingAction == BAK::HotspotAction::CHAPTER_END)
    {
        mCursor.PopCursor();
        mGuiManager.DoChapterTransition();
    }
    else if (*mPendingAction == BAK::HotspotAction::TELEPORT)
    {
        DoTeleport();
    }
    // this can be set by barding failure
    else if (*mPendingAction == BAK::HotspotAction::EXIT)
    {
        DoExit();
        return;
    }
    else if (*mPendingAction == BAK::HotspotAction::GOTO)
    {
        DoGoto();
    }

    mPendingAction.reset();
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
}

void GDSScene::DoBard()
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    assert(container && container->IsShop());
    auto& shopStats = container->GetShop();

    const auto [character, skill] = mGameState.GetPartySkill(
        BAK::SkillType::Barding, true);
    mGameState.SetActiveCharacter(character);

    // uncomment this to fix the bug in Tom's Tavern with interaction
    // between gambling and barding... :)
    //if (shopStats.mBardingMaxReward != shopStats.mBardingReward)
    if (shopStats.mBardingReward == 0)
    {
        StartDialog(BAK::DialogSources::mBardingAlreadyDone, false);
    }
    else
    {
        const auto status = BAK::Bard::ClassifyBardAttempt(
            skill, shopStats.mBardingSkill);

        switch (status)
        {
            case BAK::Bard::BardStatus::Failed:
                AudioA::GetAudioManager().ChangeMusicTrack(AudioA::BAD_BARD);
                break;
            case BAK::Bard::BardStatus::Poor:
                AudioA::GetAudioManager().ChangeMusicTrack(AudioA::POOR_BARD);
                break;
            case BAK::Bard::BardStatus::Good:
                AudioA::GetAudioManager().ChangeMusicTrack(AudioA::GOOD_BARD);
                break;
            case BAK::Bard::BardStatus::Best:
                AudioA::GetAudioManager().ChangeMusicTrack(AudioA::BEST_BARD);
                break;
        }

        const auto reward = BAK::Bard::GetReward(
            status,
            BAK::Sovereigns{shopStats.mBardingMaxReward},
            mGameState.GetChapter());

        // this is to ensure that we can rebard if we didn't succeed at first
        mGameState.SetBardReward_754d(shopStats.mBardingReward);

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

        mBarding = true;
        mGameState.SetItemValue(reward);
        StartDialog(GetDialog(status), false);
    }
}

void GDSScene::DoRepair()
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    assert(container && container->IsShop());
    mRepair.EnterRepair(container->GetShop());
}

void GDSScene::DoTeleport()
{
    // After end of dialog state...
    if (mGameState.GetMoreThanOneTempleSeen())
    {
        assert(mSceneHotspots.GetTempleNumber());
        mGuiManager.ShowTeleport(
            *mSceneHotspots.GetTempleNumber(),
            &mGameState.GetContainerForGDSScene(mReference)->GetShop());
    }
    else
    {
        StartDialog(BAK::DialogSources::mTeleportDialogNoDestinations, false);
    }
}

void GDSScene::DoTemple(BAK::KeyTarget target)
{
    auto* container = mGameState.GetContainerForGDSScene(mReference);
    ASSERT(container && container->IsShop());
    assert(mSceneHotspots.GetTempleNumber());
    const auto& scene1 = mSceneHotspots.GetScene(
        mSceneHotspots.mSceneIndex1, mGameState);
    const auto& scene2 = mSceneHotspots.GetScene(
        3, mGameState);
    AddStaticTTM(scene1, scene2);
    mDialogDisplay.Clear();
    mTemple.EnterTemple(
        target,
        *mSceneHotspots.GetTempleNumber(),
        container->GetShop(),
        this);
}

void GDSScene::DoGoto()
{
    mGuiManager.DoFade(.8, [this, pendingGoto=*mPendingGoto]{
        mGuiManager.EnterGDSScene(pendingGoto, []{});
    });
    mPendingGoto.reset();
}

void GDSScene::DoExit()
{
    mGuiManager.ExitGDSScene();
}

}
