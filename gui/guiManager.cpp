#include "gui/guiManager.hpp"

#include "gui/IDialogScene.hpp"

#include "bak/gameState.hpp"
#include "bak/startupFiles.hpp"

#include "com/cpptrace.hpp"

namespace Gui {

GuiScreen::GuiScreen(std::function<void()> finished)
:
    mFinished{finished}
{}

GuiManager::GuiManager(
    Cursor& cursor,
    Graphics::SpriteManager& spriteManager,
    BAK::GameState& gameState)
:
    Widget{
        Graphics::DrawMode::Rect,
        Graphics::SpriteSheetIndex{0},
        Graphics::TextureIndex{0},
        Graphics::ColorMode::SolidColor,
        glm::vec4{0},
        glm::vec2{0},
        glm::vec2{1},
        false
    },
    mFontManager{spriteManager},
    mActors{spriteManager},
    mBackgrounds{spriteManager},
    mIcons{spriteManager},
    mCursor{cursor},
    mGameState{gameState},
    mScreenStack{},
    mDialogRunner{
        glm::vec2{0, 0},
        glm::vec2{320, 240},
        mActors,
        mBackgrounds,
        mFontManager.GetGameFont(),
        gameState,
        mScreenStack,
        [this](const auto& choice){ DialogFinished(choice); }
    },
    mSpriteManager{spriteManager},
    mCutscenePlayer{
        spriteManager,
        mAnimatorStore,
        mFontManager.GetGameFont(),
        mFontManager.GetBookFont(),
        mBackgrounds,
        *this,
        [this](){ CutsceneFinished(); }
    },
    mMainView{*this, mBackgrounds, mIcons, mFontManager.GetSpellFont()},
    mMainMenu{*this, mSpriteManager, mBackgrounds, mIcons, mFontManager.GetGameFont()},
    mInfoScreen{
        *this,
        mActors,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mInventoryScreen{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mCampScreen{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mCastScreen{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mFontManager.GetSpellFont(),
        mGameState
    },
    mCureScreen{
        *this,
        mActors,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mLockScreen{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mFullMap{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mMoredhelScreen{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetAlienFont(),
        mFontManager.GetPuzzleFont(),
        mGameState
    },
    mTeleportScreen{
        *this,
        mBackgrounds,
        mIcons,
        mFontManager.GetGameFont(),
        mGameState
    },
    mFadeScreen{
        mAnimatorStore,
        [this]{ FadeInDone(); },
        [this]{ FadeOutDone(); }
    },
    mFadeFunction{},
    mGdsScenes{},
    mDialogScene{nullptr},
    mGuiScreens{},
    mAnimatorStore{},
    mZoneLoader{nullptr},
    mLogger{Logging::LogState::GetLogger("Gui::GuiManager")}
{
    mGdsScenes.reserve(4);
    AddChildBack(&mScreenStack);
}


[[nodiscard]] bool GuiManager::OnMouseEvent(const MouseEvent& event)
{
    if (HaveChild(&mFadeScreen))
    {
        return true;
    }
    else
    {
        return Widget::OnMouseEvent(event);
    }
}

ScreenStack& GuiManager::GetScreenStack()
{
    return mScreenStack;
}

void GuiManager::LoadGame(std::string save, std::optional<BAK::Chapter> chapter)
{
    ASSERT(mZoneLoader);
    mZoneLoader->LoadGame(save, chapter);
    mMainView.SetHeading(mGameState.GetLocation().mHeading);
}

void GuiManager::SaveGame(const BAK::SaveFile& saveFile)
{
    mGameState.Save(saveFile);
    EnterMainView();
}

void GuiManager::SetZoneLoader(BAK::IZoneLoader* zoneLoader)
{
    ASSERT(zoneLoader);
    mZoneLoader = zoneLoader;
}

void GuiManager::DoFade(double duration, std::function<void()>&& fadeFunction)
{
    CPPTRACE(mLogger.Info(), __FUNCTION__);
    
    mFadeFunction.emplace_back(std::move(fadeFunction));
    if (!HaveChild(&mFadeScreen))
    {
        AddChildBack(&mFadeScreen);
        mFadeScreen.FadeIn(duration);
    }
}

void GuiManager::PlayCutscene(
    std::vector<BAK::CutsceneAction> actions,
    std::function<void()>&& cutsceneFinished)
{
    mCutsceneFinished = std::move(cutsceneFinished);
    for (const auto& action : actions)
    {
        mCutscenePlayer.QueueAction(action);
    }
    DoFade(1.5, [this]{
        if (mScreenStack.HasChildren())
        {
            mPreviousScreen = mScreenStack.Top();
            mScreenStack.PopScreen();
        }
        mScreenStack.PushScreen(&mCutscenePlayer);
        mCutscenePlayer.Play();
    });
}

void GuiManager::CutsceneFinished()
{
    mCutsceneFinished();
}

bool GuiManager::InMainView() const
{
    return mScreenStack.size() > 0 && mScreenStack.Top() == &mMainView;
}

void GuiManager::EnterMainView()
{
    mLogger.Info() << "Entering main view\n";
    mMainView.UpdatePartyMembers(mGameState);
    DoFade(1.0, [this]{
        mScreenStack.PopScreen();
        mScreenStack.PushScreen(&mMainView);
        if (mOnEnterMainView)
        {
            mOnEnterMainView();
            mOnEnterMainView = nullptr;
        }
    });
}

void GuiManager::EnterMainMenu(bool gameRunning)
{
    DoFade(1.0, [this, gameRunning]{
        if (gameRunning)
        {
            mScreenStack.PopScreen();
        }
        mScreenStack.PushScreen(&mMainMenu);
        mMainMenu.EnterMainMenu(gameRunning);
    });
}

void GuiManager::TeleportToGDS(
    const BAK::HotspotRef& hotspot)
{
    mLogger.Debug() << __FUNCTION__ << ":" << hotspot << "\n";
    // When teleporting we need to add the "root" GDS scene to the stack
    // because it won't have been there...
    if (hotspot.mGdsNumber < 12 && hotspot.mGdsChar != 'A')
    {
        const auto rootScene = BAK::HotspotRef{hotspot.mGdsNumber, 'A'};
        mLogger.Debug() << "Teleporting to root first: " << rootScene << "\n";
        EnterGDSScene(rootScene, []{});
    }
    mLogger.Debug() << "Teleporting to child: " << hotspot << "\n";
    EnterGDSScene(hotspot, []{});
}

void GuiManager::OnTimeDelta(double delta)
{
    mAnimatorStore.OnTimeDelta(delta);
}

void GuiManager::AddAnimator(std::unique_ptr<IAnimator>&& animator)
{
    mAnimatorStore.AddAnimator(std::move(animator));
}

void GuiManager::EnterGDSScene(
    const BAK::HotspotRef& hotspot,
    std::function<void()>&& finished)
{
    mLogger.Debug() << __FUNCTION__ << ":" << hotspot << "\n";
    mCursor.PushCursor(0);

    mGdsScenes.emplace_back(
        std::make_unique<GDSScene>(
            mCursor,
            hotspot,
            mSpriteManager,
            mActors,
            mBackgrounds,
            mFontManager.GetGameFont(),
            mGameState,
            static_cast<IGuiManager&>(*this)));

    const auto song = mGdsScenes.back()->GetSceneHotspots().mSong;
    if (song != 0)
    {
        AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::MusicIndex{song});
        mGuiScreens.push(GuiScreen{
            [fin = std::move(finished)](){
                AudioA::AudioManager::Get().PopTrack();
                std::invoke(fin);
        }});
    }
    else
    {
        mGuiScreens.push(finished);
    }

    mScreenStack.PushScreen(mGdsScenes.back().get());
    mGdsScenes.back()->EnterGDSScene();
}

void GuiManager::ExitGDSScene()
{
    mLogger.Debug() << "Exiting GDS Scene" << std::endl;
    RemoveGDSScene(true);
}

void GuiManager::RemoveGDSScene(bool runFinished)
{
    ASSERT(!mGdsScenes.empty());
    mLogger.Debug() << __FUNCTION__ << " Widgets: " << mScreenStack.GetChildren() << "\n";
    mScreenStack.PopScreen();
    mCursor.PopCursor();
    mCursor.PopCursor();
    if (runFinished)
        PopAndRunGuiScreen();
    else
        PopGuiScreen();
    mLogger.Debug() << "Removed GDS Scene: " << mGdsScenes.back() << std::endl;
    mGdsScenes.pop_back();
}

void GuiManager::StartDialog(
    BAK::Target dialog,
    bool isTooltip,
    bool drawWorldFrame,
    IDialogScene* scene)
{
    mCursor.PushCursor(0);
    mDialogRunner.SetInWorldView(InMainView() || drawWorldFrame);
    mGuiScreens.push(GuiScreen{[](){
    }});

    mScreenStack.PushScreen(&mDialogRunner);
    mDialogScene = scene;
    mDialogRunner.SetDialogScene(scene);
    mDialogRunner.BeginDialog(dialog, isTooltip);
}

void GuiManager::DialogFinished(const std::optional<BAK::ChoiceIndex>& choice)
{
    ASSERT(mDialogScene);
    PopAndRunGuiScreen();
    mScreenStack.PopScreen(); // Dialog runner
    mCursor.PopCursor();

    mLogger.Debug() << "Finished dialog with choice : " << choice << "\n";
    mDialogScene->DialogFinished(choice);

    const auto teleportIndex = mDialogRunner.GetAndResetPendingTeleport();
    if (teleportIndex)
    {
        const auto& teleport = mTeleportFactory.Get(teleportIndex->mValue);
        DoTeleport(teleport);
    }

    if (mGameState.GetTransitionChapter_7541())
    {
        mGameState.SetTransitionChapter_7541(false);
        DoChapterTransition();
    }
    
    mMainView.UpdatePartyMembers(mGameState);
}

void GuiManager::DoChapterTransition()
{
    auto actions = BAK::CutsceneList::GetFinishScene(mGameState.GetChapter());
    const auto nextChapter = BAK::Chapter(mGameState.GetChapter().mValue + 1);
    for (const auto& action : BAK::CutsceneList::GetStartScene(nextChapter))
    {
        actions.emplace_back(action);
    }
    PlayCutscene(actions, [this, nextChapter]{
        // Remove gds scenes in case we transitioned from a GDS
        while (!mGdsScenes.empty())
            RemoveGDSScene();

        auto teleport = BAK::TransitionToChapter(nextChapter, mGameState);
    
        ShowGameStartMap();
        mOnEnterMainView = [this, nextChapter, teleport]{
            // Always teleport to the new world location
            const auto startLocation = LoadChapterStartLocation(nextChapter).mLocation;
            DoTeleport(
                BAK::Encounter::Teleport{
                    startLocation.mZone,
                    startLocation.mLocation,
                    std::nullopt});

            // Optionally we may need to teleport to a GDS location
            if (teleport)
            {
                mLogger.Info() << "Teleporting to: " << *teleport << std::endl;
                DoTeleport(mTeleportFactory.Get(teleport->mIndex.mValue));
            }
        };
    });
}

void GuiManager::DoTeleport(BAK::Encounter::Teleport teleport)
{
    mLogger.Info() << "Teleporting to teleport index: " << teleport << "\n";
    // Clear all stacked GDS scenes
    while (!mGdsScenes.empty())
        RemoveGDSScene();

    mLogger.Debug() << __FUNCTION__ << "Widgets: " << GetChildren() << "\n";
    if (mZoneLoader)
        mZoneLoader->DoTeleport(teleport);
    mLogger.Debug() << "Finished teleporting Widgets: " << GetChildren() << "\n";
}

void GuiManager::ShowCharacterPortrait(BAK::ActiveCharIndex character)
{
    DoFade(.8, [this, character]{
        mInfoScreen.SetSelectedCharacter(character);
        mInfoScreen.UpdateCharacter();
        mScreenStack.PushScreen(&mInfoScreen);
    });
}

void GuiManager::ExitSimpleScreen()
{
    mScreenStack.PopScreen();
}

void GuiManager::ShowInventory(BAK::ActiveCharIndex character)
{
    DoFade(.8, [this, character]{
        mCursor.PushCursor(0);
        mGuiScreens.push(GuiScreen{[](){}});
        mInventoryScreen.SetSelectionMode(false, nullptr);

        mInventoryScreen.SetSelectedCharacter(character);
        mScreenStack.PushScreen(&mInventoryScreen);
    });
}

void GuiManager::ShowContainer(BAK::GenericContainer* container, BAK::EntityType containerType)
{
    mCursor.PushCursor(0);
    ASSERT(container);
    ASSERT(container->GetInventory().GetCapacity() > 0);

    mGuiScreens.push(GuiScreen{[this, container, containerType](){
        mLogger.Debug() << "ExitContainer guiScreen hasDiag: " << container->HasDialog() << " et: " << std::to_underlying(containerType) << "\n";
        mInventoryScreen.ClearContainer();
        if (container->HasDialog())
        {
            if (containerType == BAK::EntityType::CHEST)
            {
                StartDialog(container->GetDialog().mDialog, false, false, mDialogScene);
            }
        }
    }});

    mInventoryScreen.SetSelectionMode(false, nullptr);
    mInventoryScreen.SetContainer(container, containerType);
    mLogger.Debug() << __FUNCTION__ << " Pushing inv\n";
    mScreenStack.PushScreen(&mInventoryScreen);
}

void GuiManager::SelectItem(std::function<void(std::optional<std::pair<BAK::ActiveCharIndex, BAK::InventoryIndex>>)>&& itemSelected)
{
    mCursor.PushCursor(0);

    mGuiScreens.push(GuiScreen{[&, selected=itemSelected]() mutable {
            mLogger.Debug() << __FUNCTION__ << " SelectItem\n";
            selected(std::nullopt);
    }});

    mInventoryScreen.SetSelectionMode(true, std::move(itemSelected));
    mLogger.Debug() << __FUNCTION__ << " Pushing select item\n";
    mScreenStack.PushScreen(&mInventoryScreen);
}

void GuiManager::ExitInventory()
{
    mLogger.Debug() << __FUNCTION__ << " BEGIN" << std::endl;

    auto exitInventory = [&]{
        mCursor.PopCursor();
        mScreenStack.PopScreen();
        PopAndRunGuiScreen();
    };

    if (mGameState.GetTransitionChapter_7541())
    {
        exitInventory();
        mGameState.SetTransitionChapter_7541(false);
        DoChapterTransition();
    }
    else
    {
        DoFade(1.0, [this, exitInventory]{ 
            exitInventory();
            
            mLogger.Debug() << __FUNCTION__ << " ExitInventory" << std::endl;
        });
    }
}

void GuiManager::ShowLock(
    BAK::IContainer* container,
    std::function<void()>&& finished)
{
    ASSERT(container->HasLock()
        && (container->GetLock().IsFairyChest()
            || !container->GetLock().IsTrapped()));

    mCursor.PushCursor(0);

    if (container->GetLock().IsFairyChest())
    {
        mMoredhelScreen.SetContainer(container);
        mScreenStack.PushScreen(&mMoredhelScreen);
        AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::PUZZLE_CHEST_THEME);
        mGuiScreens.push(GuiScreen{
            [fin = std::move(finished)](){
                AudioA::AudioManager::Get().PopTrack();
                std::invoke(fin);
        }});
    }
    else
    {
        mLockScreen.SetContainer(container);
        mScreenStack.PushScreen(&mLockScreen);
        mGuiScreens.push(finished);
    }
}

void GuiManager::ShowCamp(bool isInn, BAK::ShopStats* inn)
{
    assert(!isInn || inn);
    mGuiScreens.push(GuiScreen{[this]{
        mCursor.PopCursor();
    }});

    DoFade(.8, [this, isInn, inn]{
        mScreenStack.PushScreen(&mCampScreen);
        mCursor.PushCursor(0);
        mCampScreen.BeginCamp(isInn, inn);
    });
}

void GuiManager::ShowCast(bool inCombat)
{
    mGuiScreens.push(GuiScreen{[this]{
        mCursor.PopCursor();
    }});

    DoFade(.8, [this, inCombat]{
        mScreenStack.PushScreen(&mCastScreen);
        mCursor.PushCursor(0);
        mCastScreen.BeginCast(inCombat);
    });
}

void GuiManager::ShowFullMap()
{
    mFullMap.UpdateLocation();
    mFullMap.DisplayMapMode();
    DoFade(1.0, [this]{
        mScreenStack.PushScreen(&mFullMap);
    });
}

void GuiManager::ShowGameStartMap()
{
    DoFade(1.0, [this]{
        mScreenStack.PopScreen();
        mFullMap.DisplayGameStartMode(mGameState.GetChapter(), mGameState.GetMapLocation());
        mScreenStack.PushScreen(&mFullMap);
    });
}

void GuiManager::ShowCureScreen(
    unsigned templeNumber,
    unsigned cureFactor,
    std::function<void()>&& finished)
{
    DoFade(.8, [this, templeNumber, cureFactor, finished=std::move(finished)]() mutable {
        mCureScreen.EnterScreen(templeNumber, cureFactor, std::move(finished));
        mScreenStack.PushScreen(&mCureScreen);
    });
}

void GuiManager::ShowTeleport(unsigned sourceTemple, BAK::ShopStats* temple)
{
    mTeleportScreen.SetSourceTemple(sourceTemple, temple);
    DoFade(.8, [this]{
        mCursor.PopCursor();
        mScreenStack.PushScreen(&mTeleportScreen);
    });
}

void GuiManager::ExitLock()
{
    mCursor.PopCursor();
    mScreenStack.PopScreen();
    PopAndRunGuiScreen();
}

bool GuiManager::IsLockOpened() const
{
    return mLockScreen.IsUnlocked();
} 

bool GuiManager::IsWordLockOpened() const
{
    return mMoredhelScreen.IsUnlocked();
}

void GuiManager::PopGuiScreen()
{
    mGuiScreens.pop();
}

void GuiManager::PopAndRunGuiScreen()
{
    // Avoids reentrancy issue by ensuring this gui screen is not
    // in the stack when it is executed.
    auto guiScreen = mGuiScreens.top();
    mGuiScreens.pop();
    guiScreen.mFinished();
}

void GuiManager::FadeInDone()
{
    mLogger.Spam() << "FadeInDone\n";
    ASSERT(!mFadeFunction.empty());
    unsigned i = 0;
    while (!mFadeFunction.empty())
    {
        mLogger.Spam() << "Executing fade function #" << i++ << "\n";
        auto function = std::move(mFadeFunction.front());
        mFadeFunction.erase(mFadeFunction.begin());
        function();
    }
    mFadeScreen.FadeOut();
}

void GuiManager::FadeOutDone()
{
    RemoveChild(&mFadeScreen);
    if (mEndFadeFunction)
    {
        mEndFadeFunction();
        mEndFadeFunction = nullptr;
    }
}

}
