#pragma once

#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/IZoneLoader.hpp"
#include "bak/camera.hpp"
#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/state/encounter.hpp"
#include "bak/monster.hpp"
#include "bak/time.hpp"
#include "bak/types.hpp"
#include "bak/zone.hpp"

#include "com/logger.hpp"

#include "gui/guiManager.hpp"

namespace Game {

class ClickableEntity
{
public:
    BAK::EntityType mEntityType;
    BAK::GenericContainer* mContainer;
};

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
        mInteractableFactory{
            mGuiManager,
            mGameState,
            [this](const auto& pos){ CheckAndDoEncounter(pos); }},
        mCurrentInteractable{nullptr},
        mDynamicDialogScene{
            [&](){ mCamera.SetAngle(mSavedAngle); },
            [&](){ mCamera.SetAngle(mSavedAngle + glm::vec2{3.14, 0}); },
            [&](const auto&){ }
        },
        mGameData{nullptr},
        mZoneData{nullptr},
        mActiveEncounter{nullptr},
        mEncounters{},
        mClickables{},
        mNullContainer{
            BAK::ContainerHeader{},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            BAK::Inventory{0}},
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
        mLogger.Debug() << "Teleporting to: " << teleport << "\n";
        if (teleport.mTargetZone)
        {
            DoTransition(
                teleport.mTargetZone->mValue,
                teleport.mTargetLocation);
        }

        if (teleport.mTargetGDSScene)
        {
            mGuiManager.TeleportToGDS(
                *teleport.mTargetGDSScene);
        }
    }

    void LoadGame(std::string savePath) override
    {
        mGameData = std::make_unique<BAK::GameData>(savePath);
        mGameState.LoadGameData(mGameData.get());
        LoadZoneData(mGameState.GetZone().mValue);
    }

    void LoadZoneData(unsigned zone)
    {
        mZoneData = std::make_unique<BAK::Zone>(zone);
        mLoadRenderer(*mZoneData);
        LoadSystems();
        mCamera.SetGameLocation(mGameState.GetLocation());
    }

    void DoTransition(
        unsigned targetZone,
        BAK::GamePositionAndHeading targetLocation)
    {
        mGameState.SetLocation(
            BAK::Location{
                targetZone,
                BAK::GetTile(targetLocation.mPosition),
                targetLocation});

        LoadZoneData(targetZone);
    }

    void LoadSystems()
    {
        mSystems = std::make_unique<Systems>();
        mEncounters.clear();
        mClickables.clear();
        mActiveEncounter = nullptr;

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
                        const auto bakLocation = item.GetBakLocation();
                        const auto et = item.GetZoneItem().GetEntityType();
                        mSystems->AddClickable(Clickable{id});

                        auto& containers = mGameState.GetContainers(
                            BAK::ZoneNumber{mZoneData->mZoneLabel.GetZoneNumber()});
                        auto cit = std::find_if(containers.begin(), containers.end(),
                            [&bakLocation](const auto& x){
                                return x.GetHeader().GetPosition() == bakLocation;
                            });

                        if (cit != containers.end())
                        {
                            mClickables.emplace(id, ClickableEntity{et, &(*cit)});
                            continue;
                        }

                        auto fit = std::find_if(
                            mZoneData->mFixedObjects.begin(),
                            mZoneData->mFixedObjects.end(),
                            [&bakLocation](const auto& x){
                                return x.GetHeader().GetPosition() == bakLocation;
                            });

                        if (fit != mZoneData->mFixedObjects.end())
                        {
                            mClickables.emplace(id, ClickableEntity{et, &(*fit)});
                            continue;
                        }

                        mClickables.emplace(id, ClickableEntity{et, &mNullContainer});
                    }
                }
            }
        }

        const auto monsters = BAK::MonsterNames::Get();
        for (const auto& world : mZoneData->mWorldTiles.GetTiles())
        {
            for (const auto& enc : world.GetEncounters(mGameState.GetChapter()))
            {
                auto id = mSystems->GetNextItemId();
                const auto dims = enc.GetDims();
                //if (std::holds_alternative<BAK::Encounter::Combat>(enc.GetEncounter()))
                //{
                //    mSystems->AddRenderable(
                //        Renderable{
                //            id,
                //            mZoneData->mObjects.GetObject(std::string{BAK::Encounter::ToString(enc.GetEncounter())}),
                //            enc.GetLocation(),
                //            glm::vec3{0.0},
                //            glm::vec3{dims.x, 50.0, dims.y} / BAK::gWorldScale});
                //}

                mSystems->AddIntersectable(
                    Intersectable{
                        id,
                        Intersectable::Rect{
                            static_cast<double>(dims.x),
                            static_cast<double>(dims.y)},
                        enc.GetLocation()});

                // Throw the enemies onto the map...
                evaluate_if<BAK::Encounter::Combat>(enc.GetEncounter(),
                    [&](const auto& combat){
                        for (unsigned i = 0; i < combat.mCombatants.size(); i++)
                        {
                            const auto& enemy = combat.mCombatants[i];
                            auto entityId = mSystems->GetNextItemId();
                            mSystems->AddRenderable(
                                Renderable{
                                    entityId,
                                    mZoneData->mObjects.GetObject(
                                        monsters.GetMonsterAnimationFile(BAK::MonsterIndex{enemy.mMonster - 1u})),

                                    BAK::ToGlCoord<float>(enemy.mLocation.mPosition),
                                    glm::vec3{0},
                                    glm::vec3{1}});

                            auto container = std::find_if(mGameState.mCombatContainers.begin(), mGameState.mCombatContainers.end(),
                                [&](auto& lhs){
                                    return lhs.GetHeader().GetCombatNumber() == combat.mCombatIndex
                                        && lhs.GetHeader().GetCombatantNumber() == i;
                                });
                            if (container != mGameState.mCombatContainers.end())
                            {
                                mSystems->AddClickable(Clickable{entityId});
                                mClickables.emplace(entityId, ClickableEntity(BAK::EntityType::DEAD_COMBATANT, &(*container)));
                            }
                        }
                    });

                mEncounters.emplace(id, &enc);
            }
        }
    }

    void DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container)
    {
        mLogger.Debug() << __FUNCTION__ << " " 
            << static_cast<unsigned>(et) << " " << container << "\n";

        mCurrentInteractable = mInteractableFactory.MakeInteractable(et);
        ASSERT(mCurrentInteractable);
        mCurrentInteractable->BeginInteraction(container, et);
    }

    // Returns <combatActive, combatScouted>
    std::pair<bool, bool> CheckCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat)
    {
        mLogger.Info() << __FUNCTION__ << " Checking combat active\n";
        if (!mGameState.Apply(BAK::State::CheckCombatActive, encounter, mGameState.GetZone()))
        {
            mLogger.Info() << __FUNCTION__ << " Combat inactive\n";
            return std::make_pair(false, false);
        }

        if (!combat.mIsAmbush)
        {
            mLogger.Info() << __FUNCTION__ << " Combat is not ambush\n";
            return std::make_pair(true, false);
        }
        else
        {
            // This seems to be a variable used to prevent the scouting of multiple combats
            // at once...
            const auto arg_dontDoCombatIfIsAmbush = false;
            if (arg_dontDoCombatIfIsAmbush)
            {
                return std::make_pair(false, false);
            }
            else
            {
                if (!mGameState.Apply(BAK::State::CheckRecentlyEncountered, encounter.GetIndex().mValue))
                {
                    mGameState.Apply(BAK::State::SetRecentlyEncountered, encounter.GetIndex().mValue);
                    auto chance = GetRandomNumber(0, 0xfff) % 100;
                    const auto [character, scoutSkill] = mGameState.GetPartySkill(BAK::SkillType::Scouting, true);
                    mLogger.Info() << __FUNCTION__ << " Trying to scout combat: "
                        << scoutSkill << " chance: " << chance << "\n";
                    if (scoutSkill > chance)
                    {
                        mGameState.GetParty().ImproveSkillForAll(
                            BAK::SkillType::Scouting, BAK::SkillChange::ExercisedSkill, 1);
                        mGuiManager.StartDialog(
                            combat.mScoutDialog,
                            false,
                            false,
                            &mDynamicDialogScene);

                        mGameState.Apply(BAK::State::SetCombatEncounterScoutedState,
                            encounter.GetIndex().mValue, true);

                        return std::make_pair(false, true);
                    }
                    else
                    {
                        return std::make_pair(true, false);
                    }
                }
                else
                {
                    mLogger.Info() << __FUNCTION__ << " Combat was scouted already\n";
                    return std::make_pair(true, false);
                }
            }
        }
    }

    void CheckAndDoCombatEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Combat& combat)
    {
        const auto [combatActive, combatScouted] = CheckCombatEncounter(encounter, combat);

        mLogger.Debug() << __FUNCTION__ << " Combat checked, result: [" << combatActive << ", " << combatScouted << "]\n";

        if (!combatActive)
        {
            mLogger.Debug() << __FUNCTION__ << " Combat not active, not doing it\n";
            return;
        }

        const auto [character, stealthSkill] = mGameState.GetPartySkill(BAK::SkillType::Stealth, false);
        auto lowestStealth = stealthSkill;
        if (lowestStealth < 0x5a) // 90
        {
            lowestStealth *= 0x1e; // 30
            lowestStealth += (lowestStealth / 100);
        }
        if (lowestStealth > 0x5a) lowestStealth = 0x5a;

        if (combat.mIsAmbush)
        {
            if (mGameState.GetSpellActive(BAK::StaticSpells::DragonsBreath))
            {
                lowestStealth = lowestStealth + ((100 - lowestStealth) >> 1);
            }
        }

        auto chance = GetRandomNumber(0, 0xfff) % 100;
        if (lowestStealth > chance)
        {
            mGameState.GetParty().ImproveSkillForAll(
                BAK::SkillType::Stealth , BAK::SkillChange::ExercisedSkill, 1);

            mLogger.Debug() << __FUNCTION__ << " Avoided combat due to stealth\n";
            return;
        }

        // Check whether players are in valid combatable position???
        auto timeOfScouting = mGameState.Apply(BAK::State::GetCombatClickedTime, combat.mCombatIndex);
        auto timeDiff = mGameState.GetWorldTime().GetTime().mTime - timeOfScouting;
        if ((timeDiff / 0x1e) < 0x1e) // within scouting valid time
        {
            auto chance = GetRandomNumber(0, 0xfff) % 100;
            if (chance > lowestStealth)
            {
                // failed to sneak up
                mGameState.SetDialogContext(1);
            }
            else
            {
                // Successfully snuck
                mGameState.GetParty().ImproveSkillForAll(
                    BAK::SkillType::Stealth , BAK::SkillChange::ExercisedSkill, 1);
                mGameState.SetDialogContext(0);
            }
        }
        else
        {
            mGameState.SetDialogContext(2);
        }

        if (!combat.mCombatants.empty())
        {
            mGameState.mCurrentMonster = BAK::MonsterIndex{combat.mCombatants.back().mMonster};
        }

        if (combat.mEntryDialog.mValue != 0)
        {
            mGuiManager.StartDialog(
                combat.mEntryDialog,
                false,
                false,
                &mDynamicDialogScene);
        }

        // The below needs to be called in the return from the combat screen
        bool combatRetreated;
        //auto combatResult = EnterCombatScreen(surprisedEnemy, &combatRetreated);
        auto combatResult = 1;
        //recordTimeOfCombat at (combatIndex << 2) + 0x3967
        if (combatResult == 2)
        {
            // retreat to a combat retreat location based on player entry
        }
        else if (combatResult == 1)
        {
            // Victory?
            if (encounter.mSaveAddress3 != 0)
            {
                mGameState.Apply(BAK::State::SetEventFlagTrue, encounter.mSaveAddress3);
            }
        }

        mGameState.Apply(
            BAK::State::SetUniqueEncounterStateFlag,
            mGameState.GetZone(),
            encounter.GetTileIndex(),
            encounter.GetIndex().mValue,
            true);

        mGameState.Apply(
            BAK::State::SetCombatEncounterState,
            combat.mCombatIndex,
            true);

        mGameState.Apply(BAK::State::SetPostCombatCombatSpecificFlags, combat.mCombatIndex);
        // This is a part of the above function, but I separate it out here
        // to keep things cleaner. Yes this is hardcoded in the game code.
        static constexpr auto NagoCombatIndex = 74;
        if (combat.mCombatIndex == NagoCombatIndex)
        {
            // I happen to know all this "dialog" does is set a bunch of flags,
            // so it's safe to do this rather than triggering an actual dialog.
            for (const auto& action : BAK::DialogStore::Get().GetSnippet(BAK::DialogSources::mAfterNagoCombatSetKeys).mActions)
            {
                mGameState.EvaluateAction(action);
            }
        }

        if (combatResult == 3)
        {
        }
    }

    void DoBlockEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Block& block)
    {
        if (!mGameState.CheckEncounterActive(encounter))
            return;

        mGuiManager.StartDialog(
                block.mDialog,
                false,
                false,
                &mDynamicDialogScene);

        mCamera.UndoPositionChange();
        mGameState.SetPostEnableOrDisableEventFlags(encounter);
    }

    void DoEventFlagEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::EventFlag& flag)
    {
        if (!mGameState.CheckEncounterActive(encounter))
            return;

        if (flag.mEventPointer != 0)
            mGameState.SetEventValue(flag.mEventPointer, flag.mIsEnable ? 1 : 0);

        mGameState.SetPostEnableOrDisableEventFlags(encounter);
    }

    void DoZoneEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Zone& zone)
    {
        if (!mGameState.CheckEncounterActive(encounter))
            return;
        const auto& choices = BAK::DialogStore::Get().GetSnippet(zone.mDialog).mChoices;
        const bool isNoAffirmative = choices.size() == 1 
            && std::holds_alternative<BAK::QueryChoice>(choices.back().mChoice)
            && std::get<BAK::QueryChoice>(choices.back().mChoice).mQueryIndex == BAK::Keywords::sNoIndex;
        mDynamicDialogScene.SetDialogFinished(
            [&, isNoAffirmative=isNoAffirmative, zone=zone](const auto& choice){
                // These dialogs should always result in a choice
                ASSERT(choice);
                Logging::LogDebug("Game::GameRunner") << "Switch to zone: " << zone << "\n";
                if ((choice->mValue == BAK::Keywords::sYesIndex && !isNoAffirmative)
                    || (choice->mValue == BAK::Keywords::sNoIndex && isNoAffirmative))
                {
                    DoTransition(
                        zone.mTargetZone,
                        zone.mTargetLocation);
                    Logging::LogDebug("Game::GameRunner") << "Transition to: " << zone.mTargetZone << " complete\n";
                }
                else
                {
                    mCamera.UndoPositionChange();
                }
                mDynamicDialogScene.ResetDialogFinished();
            });
		mLogger.Info() << "Zone transition: " << zone << "\n";
        mGuiManager.StartDialog(
            zone.mDialog,
            false,
            false,
            &mDynamicDialogScene);
    }

    void DoDialogEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::Dialog& dialog)
    {
        if (!mGameState.CheckEncounterActive(encounter))
            return;

        mSavedAngle = mCamera.GetAngle();
        mDynamicDialogScene.SetDialogFinished(
            [&](const auto&){
                mCamera.SetAngle(mSavedAngle);
                mDynamicDialogScene.ResetDialogFinished();
            });

        mGuiManager.StartDialog(
            dialog.mDialog,
            false,
            true,
            &mDynamicDialogScene);

        if (mGameState.mGameData)
            mGameState.SetPostDialogEventFlags(
                encounter);
    }

    void DoGDSEncounter(
        const BAK::Encounter::Encounter& encounter,
        const BAK::Encounter::GDSEntry& gds)
    {
        // Pretty sure GDS encoutners will always happen...
        //if (!mGameState.CheckEncounterActive(encounter))
        //    return;

        mDynamicDialogScene.SetDialogFinished(
            [&, gds=gds](const auto& choice){
                // These dialogs should always result in a choice
                ASSERT(choice);
                if (choice->mValue == BAK::Keywords::sYesIndex)
                {
                    mGuiManager.DoFade(.8, [this, gds=gds]{
                        mGuiManager.EnterGDSScene(
                            gds.mHotspot, 
                            [&, exitDialog=gds.mExitDialog](){
                                mGuiManager.StartDialog(
                                    exitDialog,
                                    false,
                                    false,
                                    &mDynamicDialogScene);
                                });
                                mCamera.SetGameLocation(gds.mExitPosition);
                    });
                }
                else
                {
                    mCamera.UndoPositionChange();
                }

                mGameState.SetPostGDSEventFlags(encounter);
                mDynamicDialogScene.ResetDialogFinished();
            });

        mGuiManager.StartDialog(
            gds.mEntryDialog,
            false,
            false,
            &mDynamicDialogScene);
    }

    void DoEncounter(const BAK::Encounter::Encounter& encounter)
    {
        mLogger.Spam() << "Doing Encounter: " << encounter << "\n";
        std::visit(
            overloaded{
            [&](const BAK::Encounter::GDSEntry& gds){
                if (mGuiManager.mScreenStack.size() == 1)
                    DoGDSEncounter(encounter, gds);
            },
            [&](const BAK::Encounter::Block& block){
                if (mGuiManager.mScreenStack.size() == 1)
                    DoBlockEncounter(encounter, block);
            },
            [&](const BAK::Encounter::Combat& combat){
                if (mGuiManager.mScreenStack.size() == 1)
                    CheckAndDoCombatEncounter(encounter, combat);
            },
            [&](const BAK::Encounter::Dialog& dialog){
                if (mGuiManager.mScreenStack.size() == 1)
                    DoDialogEncounter(encounter, dialog);
            },
            [&](const BAK::Encounter::EventFlag& flag){
                if (mGuiManager.mScreenStack.size() == 1)
                    DoEventFlagEncounter(encounter, flag);
            },
            [&](const BAK::Encounter::Zone& zone){
                if (mGuiManager.mScreenStack.size() == 1)
                    DoZoneEncounter(encounter, zone);
            },
        },
        encounter.GetEncounter());
    }

    void CheckAndDoEncounter(glm::uvec2 position)
    {
        mLogger.Debug() << __FUNCTION__ << " Pos: " << position << "\n";
        auto intersectable = mSystems->RunIntersection(
            BAK::ToGlCoord<float>(position));
        if (intersectable)
        {
            auto it = mEncounters.find(*intersectable);
            if (it != mEncounters.end())
            {
                const auto* encounter = it->second;
                mActiveEncounter = encounter;
                if (mActiveEncounter)
                    DoEncounter(*mActiveEncounter);
            }
        }
    }

    void RunGameUpdate()
    {
        if (mCamera.CheckAndResetDirty())
        {
            // This class var is only really necessary so that
            // we can us IMGUI to pull the latest triggered interactable.
            // Might want to clean this up.
            mActiveEncounter = nullptr;

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

            if (auto unitsTravelled = mCamera.GetAndClearUnitsTravelled(); unitsTravelled > 0)
            {
                auto camp = BAK::TimeChanger{mGameState};
                camp.ElapseTimeInMainView(
                    BAK::Time{0x1e * unitsTravelled});
            }

            if (mActiveEncounter)
            {
                DoEncounter(*mActiveEncounter);
            }
        }
    }

    void CheckClickable(unsigned entityId)
    {
        assert(mSystems);

        auto bestId = std::optional<BAK::EntityIndex>{};
        for (const auto& entity : mSystems->GetClickables())
        {
            if (entity.GetId().mValue == entityId)
            {
                bestId = entity.GetId();
                break;
            }
        }
        mLogger.Debug() << "Checked clickable entity id: " << entityId
            << " found: " << bestId << "\n";

        if (bestId)
        {
            auto it = mClickables.find(*bestId);
            assert (it != mClickables.end());
            auto& clickable = mClickables[*bestId];
            assert(clickable.mContainer);
            DoGenericContainer(clickable.mEntityType, *clickable.mContainer);
        }
    }

    Camera& mCamera;
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    InteractableFactory mInteractableFactory;
    std::unique_ptr<IInteractable> mCurrentInteractable;
    Gui::DynamicDialogScene mDynamicDialogScene;

    std::unique_ptr<BAK::GameData> mGameData;
    std::unique_ptr<BAK::Zone> mZoneData;

    const BAK::Encounter::Encounter* mActiveEncounter;
    std::unordered_map<BAK::EntityIndex, const BAK::Encounter::Encounter*> mEncounters;
    std::unordered_map<BAK::EntityIndex, ClickableEntity> mClickables{};
    BAK::GenericContainer mNullContainer;
    std::unique_ptr<Systems> mSystems;
    glm::vec2 mSavedAngle;
    std::function<void(const BAK::Zone&)> mLoadRenderer;
    BAK::Encounter::TeleportFactory mTeleportFactory;

    bool mClickablesEnabled;

    const Logging::Logger& mLogger;

};


}
