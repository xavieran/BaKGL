#pragma once

#include "game/systems.hpp"

#include "bak/IZoneLoader.hpp"
#include "bak/camera.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/zone.hpp"

#include "gui/guiManager.hpp"


namespace Game {

class GameRunner : public BAK::IZoneLoader
{
public:
    GameRunner(
        Camera& camera,
        BAK::GameState& gameState,
        Gui::GuiManager& guiManager,
        std::function<void(const BAK::Zone&)>&& loadRenderer)
    :
        mCamera{camera},
        mGameState{gameState},
        mGuiManager{guiManager},
        mDynamicDialogScene{
            [&](){ mCamera.SetAngle(mSavedAngle); },
            [&](){ mCamera.SetAngle(mSavedAngle + glm::vec2{3.14, 0}); },
            [&](const auto&){ }
        },
        mZoneData{nullptr},
        mActiveEncounter{nullptr},
        mActiveClickable{nullptr},
        mActiveClickable2{nullptr},
        mEncounters{},
        mClickables{},
        mSystems{nullptr},
        mSavedAngle{0},
        mLoadRenderer{std::move(loadRenderer)},
        mTeleportFactory{},
        mClickablesEnabled{false},
        mLogger{Logging::LogState::GetLogger("Game::GameRunner")}
    {
        ASSERT(mLoadRenderer);
    }

    void DoTeleport(BAK::TeleportIndex teleIndex) override
    {
        const auto& teleport = mTeleportFactory.Get(teleIndex.mValue);
        if (teleport.mTargetZone)
            DoTransition(
                teleport.mTargetZone->mValue,
                teleport.mTargetLocation);

        if (teleport.mTargetGDSScene)
            mGuiManager.TeleportToGDS(*teleport.mTargetGDSScene);
    }

    void LoadZoneData(unsigned zone)
    {
        mZoneData = std::make_unique<BAK::Zone>(zone);
        LoadSystems();
        mLoadRenderer(*mZoneData);
    }

    void DoTransition(
        unsigned targetZone,
        BAK::GamePositionAndHeading targetLocation)
    {
        LoadZoneData(targetZone);
        mCamera.SetGameLocation(targetLocation);
    }

    void LoadSystems()
    {
        mSystems = std::make_unique<Systems>();
        mEncounters.clear();
        mClickables.clear();
        mActiveEncounter = nullptr;
        mActiveClickable = nullptr;
        mActiveClickable2 = nullptr;

        for (const auto& world : mZoneData->mWorldTiles.GetTiles())
        {
            for (const auto& item : world.GetItems())
            {
                if (item.GetZoneItem().GetVertices().size() > 1)
                {
                    auto id = mSystems->GetNextItemId();
                    auto renderable = Renderable{
                        id,
                        mZoneData->mObjects.GetObject(item.GetZoneItem().GetName()),
                        item.GetLocation(),
                        item.GetRotation(),
                        glm::vec3{static_cast<float>(item.GetZoneItem().GetScale())}};

                    if (item.GetZoneItem().IsSprite())
                        mSystems->AddSprite(renderable);
                    else
                        mSystems->AddRenderable(renderable);

                    if (item.GetZoneItem().GetClickable())
                    {
                        mSystems->AddClickable(
                            Clickable{
                                id,
                                250,
                                item.GetLocation()});
                        mClickables.emplace(id, &item);
                        //mSystems->AddRenderable(
                        //    Renderable{
                        //        id,
                        //        mZoneData->mObjects.GetObject("clickable"),
                        //        item.GetLocation(),
                        //        glm::vec3{1.0},
                        //        glm::vec3{1.0}});

                    }
                }
            }
        }

        for (const auto& world : mZoneData->mWorldTiles.GetTiles())
        {
            for (const auto& enc : world.GetEncounters(mGameState.GetChapter()))
            {
                auto id = mSystems->GetNextItemId();
                const auto dims = enc.GetDims();
                //mSystems->AddRenderable(
                //    Renderable{
                //        id,
                //        mZoneData->mObjects.GetObject(std::string{BAK::Encounter::ToString(enc.GetEncounter())}),
                //        enc.GetLocation(),
                //        glm::vec3{0.0},
                //        glm::vec3{dims.x, 50.0, dims.y} / BAK::gWorldScale});

                mSystems->AddIntersectable(
                    Intersectable{
                        id,
                        Intersectable::Rect{
                            static_cast<double>(dims.x),
                            static_cast<double>(dims.y)},
                        enc.GetLocation()});

                mEncounters.emplace(id, &enc);
            }
        }
    }

    void RunGameUpdate()
    {
        mActiveEncounter = nullptr;
        //mActiveClickable = nullptr;

        auto intersectable = mSystems->RunIntersection(mCamera.GetPosition());
        if (intersectable)
        {
            auto it = mEncounters.find(*intersectable);
            if (it != mEncounters.end())
            {
                const auto* encounter = it->second;
                mActiveEncounter = encounter;
            }
        }

        if (mActiveEncounter)
        {
            const auto& encounter = mActiveEncounter->GetEncounter();
            std::visit(
                overloaded{
                    [&](const BAK::Encounter::GDSEntry& gds){
                        if (mGuiManager.mScreenStack.size() == 1)
                        {
                            mDynamicDialogScene.SetDialogFinished(
                                [&, gds=gds](const auto& choice){
                                    // These dialogs should always result in a choice
                                    ASSERT(choice);
                                    if (choice->mValue == BAK::Keywords::sYesIndex)
                                    {
                                        mGuiManager.EnterGDSScene(
                                            gds.mHotspot, 
                                            [&, exitDialog=gds.mExitDialog](){
                                                mGuiManager.StartDialog(
                                                    exitDialog,
                                                    false,
                                                    false,
                                                    &mDynamicDialogScene);
                                                });
                                    }

                                    // FIXME: Move this into the if block so we only
                                    // modify the players position if they entered the town.
                                    // Will need a "TryMoveCamera" step that checks encounter 
                                    // bounds and doesn't move if its a no. 
                                    // Will need the same for block and Zone transitions too.
                                    mCamera.SetGameLocation(gds.mExitPosition);
                                    mDynamicDialogScene.ResetDialogFinished();
                                });

                            mGuiManager.StartDialog(
                                gds.mEntryDialog,
                                false,
                                false,
                                &mDynamicDialogScene);
                        }
                    },
                    [&](const BAK::Encounter::Block& e){
                        if (mGuiManager.mScreenStack.size() == 1)
                            mGuiManager.StartDialog(
                                e.mDialog,
                                false,
                                false,
                                &mDynamicDialogScene);
                    },
                    [&](const BAK::Encounter::Combat& e){
                    },
                    [&](const BAK::Encounter::Dialog& e){
                        if (mGuiManager.mScreenStack.size() == 1
                            && mGameState.mGameData->CheckActive(
                                *mActiveEncounter,
                                mGameState.GetZone()))
                        {
                            mSavedAngle = mCamera.GetAngle();
                            mDynamicDialogScene.SetDialogFinished(
                                [&](const auto&){
                                    mCamera.SetAngle(mSavedAngle);
                                    mDynamicDialogScene.ResetDialogFinished();
                                });

                            mGuiManager.StartDialog(
                                e.mDialog,
                                false,
                                true,
                                &mDynamicDialogScene);

                            if (mGameState.mGameData)
                                mGameState.mGameData->SetPostDialogEventFlags(
                                    *mActiveEncounter);
                        }

                    },
                    [](const BAK::Encounter::EventFlag& flag){
                        //if (EncoutnerActive)
                        //mGameState.SetEventState(flag.mEventPointer, flag.mIsEnable);
                    },
                    [&](const BAK::Encounter::Zone& zone){
                        if (mGuiManager.mScreenStack.size() == 1)
                        {
                            mDynamicDialogScene.SetDialogFinished(
                                [&, zone=zone](const auto& choice){
                                    // These dialogs should always result in a choice
                                    ASSERT(choice);
                                    Logging::LogDebug("Game::GameRunner") << "Switch to zone: " << zone << "\n";
                                    if (choice->mValue == BAK::Keywords::sYesIndex)
                                    {
                                        DoTransition(
                                            zone.mTargetZone,
                                            zone.mTargetLocation);
                                        Logging::LogDebug("Game::GameRunner") << "Transition to: " << zone.mTargetZone << " complete\n";
                                    }
                                    mDynamicDialogScene.ResetDialogFinished();
                                });

                            mGuiManager.StartDialog(
                                zone.mDialog,
                                false,
                                false,
                                &mDynamicDialogScene);
                        }
                    },
                },
                encounter);
        }

        if (mActiveClickable2)
        {
            const auto bakLocation = mActiveClickable2->GetBakLocation();

            auto& containers = mGameState.GetContainers(
                BAK::ZoneNumber{mZoneData->mZoneLabel.GetZoneNumber()});
            auto cit = std::find_if(containers.begin(), containers.end(),
                [&bakLocation](const auto& x){
                    return x.mLocation == bakLocation;
                });

            if (cit != containers.end())
            {
                mLogger.Debug() << "Container: " << *cit << "\n";
                if (mGuiManager.mScreenStack.size() == 1)
                {
                    if (cit->mDialog != BAK::Target{BAK::KeyTarget{0}})
                    {
                        mDynamicDialogScene.SetDialogFinished(
                            [&, cit=cit](const auto& choice){
                                if (!choice
                                    || choice->mValue == BAK::Keywords::sYesIndex)
                                {
                                    mGuiManager.ShowContainer(&(*cit));
                                }
                                mDynamicDialogScene.ResetDialogFinished();
                            });

                        mGuiManager.StartDialog(
                            cit->mDialog,
                            false,
                            //FIXME: There are a few dialogs of fixed objects which require the frame
                            false, 
                            &mDynamicDialogScene);
                    }
                    else
                    {
                        mGuiManager.ShowContainer(&(*cit));
                    }
                }
            }
            else
            {

                auto fit = std::find_if(
                    mZoneData->mFixedObjects.begin(),
                    mZoneData->mFixedObjects.end(),
                    [&bakLocation](const auto& x){ return x.mHeader.GetPosition() == bakLocation; });
                if (fit != mZoneData->mFixedObjects.end())
                {
                    if (mGuiManager.mScreenStack.size() == 1)
                    {
                        mDynamicDialogScene.SetDialogFinished(
                            [&, obj=fit](const auto&){
                                Logging::LogDebug(__FUNCTION__) << "DialogFinished: HS: " << obj->mHotspotRef << "\n";
                                if (obj->mHotspotRef)
                                {
                                    mGuiManager.EnterGDSScene(*obj->mHotspotRef, []{});
                                }
                                mDynamicDialogScene.ResetDialogFinished();
                            });
                        if (fit->mDialogKey != BAK::Target{BAK::KeyTarget{0}})
                            mGuiManager.StartDialog(
                                fit->mDialogKey,
                                false,
                                //FIXME: There are a few dialogs of fixed objects which require the frame
                                false, 
                                &mDynamicDialogScene);
                        Logging::LogDebug(__FUNCTION__) << "ClickableFixedObject: " << *fit << "\n";
                    }
                }
            }

            mActiveClickable2 = nullptr;
        }
    }

    void ResetClickable()
    {
        mActiveClickable = nullptr;
    }

    void CheckClickable()
    {
        const auto bestId = mSystems->RunClickable(
            std::make_pair(
                mCamera.GetPosition(), 
                mCamera.GetPosition() + (mCamera.GetDirection() * 300.0f)));

        if (bestId)
        {
            mActiveClickable = mClickables[*bestId];
            mActiveClickable2 = mClickables[*bestId];
        }
    }

    Camera& mCamera;
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    Gui::DynamicDialogScene mDynamicDialogScene;

    std::unique_ptr<BAK::Zone> mZoneData;

    const BAK::Encounter::Encounter* mActiveEncounter;
    const BAK::WorldItemInstance* mActiveClickable;
    const BAK::WorldItemInstance* mActiveClickable2;
    std::unordered_map<BAK::EntityIndex, const BAK::Encounter::Encounter*> mEncounters;
    std::unordered_map<BAK::EntityIndex, const BAK::WorldItemInstance*> mClickables{};
    std::unique_ptr<Systems> mSystems;
    glm::vec2 mSavedAngle;
    std::function<void(const BAK::Zone&)>&& mLoadRenderer;
    BAK::Encounter::TeleportFactory mTeleportFactory;

    bool mClickablesEnabled;

    const Logging::Logger& mLogger;

};


}
