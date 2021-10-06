#pragma once

#include "game/systems.hpp"

#include "bak/camera.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/zone.hpp"

#include "gui/guiManager.hpp"


namespace Game {

class GameRunner
{
public:
    GameRunner(
        unsigned zone,
        Camera& camera,
        BAK::GameState& gameState,
        Gui::GuiManager& guiManager)
    :
        mCamera{camera},
        mGameState{gameState},
        mGuiManager{guiManager},
        mDynamicDialogScene{
            [&](){ mCamera.SetAngle(glm::vec2{3.14, 0}); },
            [&](){ mCamera.SetAngle(glm::vec2{0}); },
            [&](){ }
        },
        mZoneLabel{zone},
        mZoneData{zone},
        mActiveEncounter{nullptr},
        mActiveClickable{nullptr},
        mEncounters{},
        mClickables{},
        mSystems{}
    {
        for (const auto& world : mZoneData.mWorldTiles.GetTiles())
        {
            for (const auto& item : world.GetItems())
            {
                if (item.GetZoneItem().GetVertices().size() > 1)
                {
                    auto id = mSystems.GetNextItemId();
                    auto renderable = Renderable{
                        id,
                        mZoneData.mObjects.GetObject(item.GetZoneItem().GetName()),
                        item.GetLocation(),
                        item.GetRotation(),
                        glm::vec3{item.GetZoneItem().GetScale()}};

                    if (item.GetZoneItem().IsSprite())
                        mSystems.AddSprite(renderable);
                    else
                        mSystems.AddRenderable(renderable);

                    if (item.GetZoneItem().GetClickable())
                    {
                        mSystems.AddClickable(
                            Clickable{
                                id,
                                500,
                                item.GetLocation()});
                        mClickables.emplace(id, &item);
                        /*mSystems.AddRenderable(
                            Renderable{
                                id,
                                objectStore.GetObject("clickable"),
                                item.GetLocation(),
                                item.GetRotation(),
                                glm::vec3{item.GetZoneItem().GetScale()}});*/
                    }
                }
            }
        }

        for (const auto& world : mZoneData.mWorldTiles.GetTiles())
        {
            for (const auto& enc : world.GetEncounters())
            {
                auto id = mSystems.GetNextItemId();
                const auto dims = enc.GetDims();
                mSystems.AddRenderable(
                    Renderable{
                        id,
                        mZoneData.mObjects.GetObject(std::string{BAK::Encounter::ToString(enc.GetEncounter())}),
                        enc.GetLocation(),
                        glm::vec3{0.0},
                        glm::vec3{dims.x, 50.0, dims.y} / BAK::gWorldScale});

                mSystems.AddIntersectable(
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

    void StartDialog(BAK::Target dialog, bool isTooltip)
    {

    }

    void RunGameUpdate()
    {
        mActiveEncounter = nullptr;
        //mActiveClickable = nullptr;

        auto intersectable = mSystems.RunIntersection(mCamera.GetPosition());
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
                            // mGuiManager.MakeChoice()
                            // if (yes)
                            //  EnterGDSScene()
                            //  DialogRunner.PushExitDialog
                            //mGuiManager.StartDialog(
                            //    gds.mEntryDialog,
                            //    false,
                            //    &mDynamicDialogScene);
                            //mDynamicDialogScene.SetDialogFinished(
                            //    [&](){
                            //        mDynamicDialogScene.SetDialogFinished([]{});
                            //        mGuiManager.StartDialog(
                            //            gds.mEntryDialog,
                            //            false,
                            //            &mDynamicDialogScene);
                            //    });
                        }

                        if (mGuiManager.mScreenStack.size() == 1)
                            mGuiManager.EnterGDSScene(gds.mHotspot);
                    },
                    [&](const BAK::Encounter::Block& e){
                        if (mGuiManager.mScreenStack.size() == 1)
                            mGuiManager.StartDialog(
                                e.mDialog,
                                false,
                                &mDynamicDialogScene);
                    },
                    [&](const BAK::Encounter::Combat& e){
                    },
                    [&](const BAK::Encounter::Dialog& e){
                        if (mGuiManager.mScreenStack.size() == 1
                            && mGameState.mGameData->CheckActive(
                                *mActiveEncounter,
                                mGameState.GetZone(),
                                0))
                        {
                            mGuiManager.StartDialog(
                                e.mDialog,
                                false,
                                &mDynamicDialogScene);

                            if (mGameState.mGameData)
                                mGameState.mGameData->SetPostDialogEventFlags(
                                    *mActiveEncounter);
                        }

                    },
                    [](const BAK::Encounter::EventFlag&){
                    },
                    [&](const BAK::Encounter::Zone& e){
                        if (mGuiManager.mScreenStack.size() == 1)
                            mGuiManager.StartDialog(
                                e.mDialog,
                                false,
                                &mDynamicDialogScene);
                    },
                },
                encounter);
        }

        if (mActiveClickable)
        {
            const auto bakLocation = mActiveClickable->GetBakLocation();

            //const auto containers = 
            //auto cit = std::find_if(containers.begin(), containers.end(),
            //    [&bakLocation](const auto& x){ return x.mLocation == bakLocation; });
            //if (cit != containers.end())
            //    ShowContainerGui(*cit);
            //ShowDialogGui(fit->mDialogKey, dialogStore, gameData);

            auto fit = std::find_if(mZoneData.mFixedObjects.begin(), mZoneData.mFixedObjects.end(),
                [&bakLocation](const auto& x){ return x.mLocation == bakLocation; });
            if (fit != mZoneData.mFixedObjects.end())
            {
                if (mGuiManager.mScreenStack.size() == 1)
                {
                    mDynamicDialogScene.SetDialogFinished(
                        [&, obj=fit](){
                            Logging::LogDebug(__FUNCTION__) << "DialogFinished: " << obj->mHotspotRef << "\n";
                            if (obj->mHotspotRef)
                            {
                                mGuiManager.EnterGDSScene(*obj->mHotspotRef);
                            }
                            mDynamicDialogScene.SetDialogFinished([]{});
                        });

                    mGuiManager.StartDialog(
                        fit->mDialogKey,
                        false,
                        &mDynamicDialogScene);
                }
            }

            mActiveClickable = nullptr;
        }
    }

    void CheckClickable()
    {
        const auto bestId = mSystems.RunClickable(
            std::make_pair(
                mCamera.GetPosition(), 
                mCamera.GetPosition() + (mCamera.GetDirection() * 300.0f)));

        if (bestId)
            mActiveClickable = mClickables[*bestId];
    }

    Camera& mCamera;
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    Gui::DynamicDialogScene mDynamicDialogScene;


    BAK::ZoneLabel mZoneLabel;
    BAK::Zone mZoneData;

    const BAK::Encounter::Encounter* mActiveEncounter;
    const BAK::WorldItemInstance* mActiveClickable;
    std::unordered_map<BAK::EntityIndex, const BAK::Encounter::Encounter*> mEncounters;
    std::unordered_map<BAK::EntityIndex, const BAK::WorldItemInstance*> mClickables{};
    Systems mSystems;

};


}
