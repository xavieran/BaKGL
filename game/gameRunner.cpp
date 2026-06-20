#include "game/gameRunner.hpp"

#include "bak/combat/combatModel.hpp"
#include "bak/entityType.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/camera.hpp"
#include "bak/combat/mechanics.hpp"
#include "bak/chapterTransitions.hpp"
#include "bak/encounter/combat.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/monster.hpp"
#include "bak/state/encounter.hpp"
#include "bak/time.hpp"
#include "bak/types.hpp"
#include "bak/zone.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/bits.hpp"
#include "com/ostream.hpp"
#include "com/scopeGuard.hpp"

#include "game/combatModelLoader.hpp"

#include "graphics/glm.hpp"
#include "gui/guiManager.hpp"

#include <utility>
#include <variant>

namespace Game {

GameRunner::GameRunner(
    Camera& camera,
    BAK::GameState& gameState,
    Gui::GuiManager& guiManager,
    bool debugRenderEncounters)
:
    mCamera{camera},
    mGameState{gameState},
    mGuiManager{guiManager},
    mInteractableFactory{
        mGuiManager,
        mGameState,
        [this](const auto& pos) -> bool { return CheckAndDoEncounter(pos); }},
    mCurrentInteractable{nullptr},
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
        std::nullopt,
        BAK::Inventory{0}},
    mCombatActorStore{
        mCombatModelLoader,
        nullptr
    },
    mWorldActorStore{
        mCombatModelLoader,
        nullptr
    },
    mEncounterHandler{
        mGameState,
        mGuiManager,
        mCamera},
    mClickablesEnabled{false},
    mDebugRenderEncounters{debugRenderEncounters},
    mLogger{Logging::LogState::GetLogger("Game::GameRunner")}
{
    mGameState.SetFindEncounterCallback(
        [this](BAK::CombatIndex combatIndex) -> const BAK::Encounter::Encounter& {
            return FindEncounterByCombatIndex(combatIndex);
        });
    mEncounterHandler.SetTransitionCallback(
        [this](BAK::ZoneNumber targetZone, BAK::GamePositionAndHeading targetLocation){
            DoTransition(targetZone, targetLocation);
        });
    mEncounterHandler.GetCombatHandler().SetEnterCombatCallback(
        [this](){ EnterCombatFromEncounter(); });

    mGuiManager.SetCombatManager(mCombatManager);
}

void GameRunner::DoTeleport(BAK::Encounter::Teleport teleport)
{
    mLogger.Debug() << "Teleporting to: " << teleport << "\n";
    if (teleport.mTargetZone)
    {
        DoTransition(
            *teleport.mTargetZone,
            teleport.mTargetLocation);
    }

    if (teleport.mTargetGDSScene)
    {
        mGuiManager.TeleportToGDS(
            *teleport.mTargetGDSScene);
    }
}

void GameRunner::LoadGame(std::string savePath, std::optional<BAK::Chapter> chapter)
{
    mGameState.LoadGame(savePath);
    if (chapter)
    {
        BAK::TransitionToChapter(*chapter, mGameState);
    }
    LoadZoneData(mGameState.GetZone());
}

void GameRunner::LoadZoneData(BAK::ZoneNumber zone)
{
    mZoneData = std::make_unique<BAK::Zone>(zone.mValue);
    mZoneRenderData = std::make_unique<Graphics::RenderData>();
    mZoneRenderData->LoadData(
        mZoneData->mObjects, mZoneData->mZoneTextures.GetTextures(),
        mZoneData->mZoneTextures.GetMaxDim());
    LoadSystems();
    mCamera.SetGameLocation(mGameState.GetLocation());
}

void GameRunner::DoTransition(
    BAK::ZoneNumber targetZone,
    BAK::GamePositionAndHeading targetLocation)
{
    CleanCombatsOnNewZone();
    mGameState.SetLocation(
        BAK::Location{
            targetZone,
            BAK::GetTile(targetLocation.mPosition),
            targetLocation});

    LoadZoneData(targetZone);
}

void GameRunner::LoadSystems()
{
    mSystems = std::make_unique<Systems>();
    mCombatActorStore.SetSystems(mSystems.get());
    mWorldActorStore.SetSystems(mSystems.get());
    mEncounters.clear();
    mClickables.clear();
    mActiveEncounter = nullptr;

    std::vector<glm::uvec2> handledLocations{};

    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        for (const auto& item : world.GetItems())
        {
            if (item.GetZoneItem().GetVertices().size() > 1)
            {
                auto id = mSystems->GetNextItemId();
                auto rotation = item.GetZoneItem().IsSprite() ? Graphics::sNinetyDegreeRotation : item.GetRotation();
                auto renderable = Renderable{
                    id,
                    mZoneData->mObjects.GetObject(item.GetZoneItem().GetName()),
                    item.GetLocation(),
                    rotation,
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
                        [&](const auto& x){
                            return x.GetHeader().GetPosition() == bakLocation
                                && x.GetHeader().PresentInChapter(mGameState.GetChapter());
                        });

                    if (cit != containers.end())
                    {
                        mClickables.emplace(id, ClickableEntity{et, &(*cit)});
                        handledLocations.emplace_back(item.GetBakLocation());
                        continue;
                    }

                    auto fit = std::find_if(
                        mZoneData->mFixedObjects.begin(),
                        mZoneData->mFixedObjects.end(),
                        [&](const auto& x){
                            return x.GetHeader().GetPosition() == bakLocation
                                && x.GetHeader().PresentInChapter(mGameState.GetChapter());
                        });

                    if (fit != mZoneData->mFixedObjects.end())
                    {
                        mClickables.emplace(id, ClickableEntity{et, &(*fit)});
                        handledLocations.emplace_back(item.GetBakLocation());
                        continue;
                    }

                    mClickables.emplace(id, ClickableEntity{et, &mNullContainer});
                }
            }
        }
    }

    auto& containers = mGameState.GetContainers(
        BAK::ZoneNumber{mZoneData->mZoneLabel.GetZoneNumber()});
    for (auto& container : containers)
    {
        const auto& header = container.GetHeader();
        const auto bakPosition = header.GetPosition();
        if ((std::find(handledLocations.begin(), handledLocations.end(), bakPosition) != handledLocations.end())
            || !header.PresentInChapter(mGameState.GetChapter()))
        {
            continue;
        }
        handledLocations.emplace_back(bakPosition);
        mLogger.Spam() << "Hidden container found: " << container << "\n";

        const auto id = mSystems->GetNextItemId();
        const auto& item = mZoneData->mZoneItems.GetZoneItem(header.GetModel());
        const auto location = BAK::ToGlCoord<float>(bakPosition);
        auto renderable = Renderable{
            id,
            mZoneData->mObjects.GetObject(item.GetName()),
            location,
            glm::vec3{},
            glm::vec3{static_cast<float>(item.GetScale())}};
        mSystems->AddRenderable(renderable);

        mSystems->AddClickable(Clickable{id});
        mClickables.emplace(id, ClickableEntity(BAK::EntityTypeFromModelName(item.GetName()), &container));
    }

    auto& fixedObjects = mZoneData->mFixedObjects;
    for (auto& container : fixedObjects)
    {
        const auto& header = container.GetHeader();
        const auto bakPosition = header.GetPosition();
        if ((std::find(handledLocations.begin(), handledLocations.end(), bakPosition) != handledLocations.end())
            || !header.PresentInChapter(mGameState.GetChapter()))
        {
            continue;
        }
        handledLocations.emplace_back(bakPosition);
        mLogger.Debug() << "Hidden container found: " << container << "\n";

        const auto id = mSystems->GetNextItemId();
        const auto& item = mZoneData->mZoneItems.GetZoneItem(header.GetModel());
        const auto location = BAK::ToGlCoord<float>(bakPosition);
        auto renderable = Renderable{
            id,
            mZoneData->mObjects.GetObject(item.GetName()),
            location,
            glm::vec3{},
            glm::vec3{static_cast<float>(item.GetScale())}};
        mSystems->AddRenderable(renderable);

        mSystems->AddClickable(Clickable{id});
        mClickables.emplace(id, ClickableEntity(BAK::EntityTypeFromModelName(item.GetName()), &container));
    }

    const auto monsters = BAK::MonsterNames::Get();
    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        for (const auto& enc : world.GetEncounters(mGameState.GetChapter()))
        {
            auto id = mSystems->GetNextItemId();
            const auto dims = enc.GetDims();
            if (mDebugRenderEncounters)
            {
                mSystems->AddRenderable(
                    Renderable{
                        id,
                        mZoneData->mObjects.GetObject(std::string{BAK::Encounter::ToString(enc.GetEncounter())}),
                        enc.GetLocation(),
                        glm::vec3{0.0},
                        glm::vec3{dims.x, 50.0, dims.y} / BAK::gWorldScale});
            }

            mSystems->AddIntersectable(
                Intersectable{
                    id,
                    Intersectable::Rect{
                        static_cast<double>(dims.x),
                        static_cast<double>(dims.y)},
                    enc.GetLocation()});
            mEncounters.emplace(id, &enc);
        }

        LoadTileActors(world.GetTileIndex());
    }

    mGridVisible = false;
    mGridCellRenderables.clear();
    mGridCellEntityIds.clear();
}

void GameRunner::LoadTileActors(std::uint8_t tileIndex)
{
    const auto& tile = mZoneData->mWorldTiles.GetTiles()[tileIndex];
    for (const auto& encounter : tile.GetEncounters(mGameState.GetChapter()))
    {
        if (!std::holds_alternative<BAK::Encounter::Combat>(encounter.GetEncounter()))
        {
            continue;
        }
        auto entityIndices = std::vector<BAK::EntityIndex>{};

        const auto combatComplete = !BAK::State::CheckCombatActive(mGameState, encounter, mGameState.GetZone());
        const auto& combat = std::get<BAK::Encounter::Combat>(encounter.GetEncounter());
        const auto tilePos = tile.GetTile() * static_cast<unsigned>(64000);
        for (unsigned i = 0; i < combat.mCombatants.size(); i++)
        {
            auto& cwl = mGameState.GetCombatWorldLocation(tileIndex, encounter.GetTileCombatIndex(), i);
            const auto& combatant = combat.mCombatants[i];
            if (!cwl.IsLoaded())
            {
                cwl.mImageIndex = 0;
                cwl.mState = static_cast<BAK::CombatantWorldState>(combatant.mMovementType);
                cwl.mPosition = combatant.mLocation;
            }
            auto combatantDead = cwl.mState == BAK::CombatantWorldState::Dead;

            // Combatants that aren't dead in a completed combat should not be shown
            if (combatComplete && !combatantDead)
            {
                continue;
            }

            if (!mCombatModelLoader.mCombatModelDatas[combatant.mMonster])
            {
                mLogger.Error() << "Couldn't load combat model: " << combatant.mMonster << "\n";
                continue;
            }

            auto worldPos = cwl.mPosition.mPosition + tilePos;

            mLogger.Info() << "Combatant @" << cwl << " placed at pos: " << worldPos << "\n";


            auto entityId = mWorldActorStore.AddActor(worldPos, BAK::MonsterIndex{combatant.mMonster});
            entityIndices.push_back(entityId);
            auto& actor = *mWorldActorStore.GetActor(entityId);
            if (combatantDead)
            {
                actor.SetState(BAK::AnimationType::Dead, BAK::Direction::South);
            }
            else
            {
                actor.Update();
            }
            mSystems->AddDynamicRenderable(
                DynamicRenderable{
                    entityId,
                    &actor.GetRenderData(),
                    &actor.mObject,
                    &actor.mLocation,
                    &actor.mModelMatrix});

            auto& containers = mGameState.GetCombatContainers();
            auto container = std::find_if(containers.begin(), containers.end(),
                [&](auto& lhs){
                    return lhs.GetHeader().GetCombatNumber() == combat.mCombatIndex
                        && lhs.GetHeader().GetCombatantNumber() == i;
                });

            if (container != containers.end())
            {
                const auto entityType = combatComplete ? BAK::EntityType::DEAD_COMBATANT : BAK::EntityType::LIVING_COMBATANT;
                mSystems->AddClickable(Clickable{entityId});
                mClickables.emplace(entityId, ClickableEntity(entityType, &(*container)));
            }
        }

        mCombatActorIds.emplace(combat.mCombatIndex.mValue, std::move(entityIndices));
    }
}

void GameRunner::UnloadWorldActors()
{
    for (const auto& [combatIndex, entityIds] : mCombatActorIds)
    {
        for (auto entityId : entityIds)
        {
            mClickables.erase(entityId);
            mSystems->RemoveClickable(entityId);
        }
    }
    mCombatActorIds.clear();
    mWorldActorStore.Clear();
}

void GameRunner::LoadWorldActors()
{
    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        LoadTileActors(world.GetTileIndex());
    }
}

void GameRunner::ClearCombatActors()
{
    mCombatActorStore.Clear();
    mCombatManager.Clear();
}

void GameRunner::DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container)
{
    mLogger.Debug() << __FUNCTION__ << " " 
        << static_cast<unsigned>(et) << " " << container << "\n";

    mCurrentInteractable = mInteractableFactory.MakeInteractable(et);
    ASSERT(mCurrentInteractable);
    mCurrentInteractable->BeginInteraction(container, et);
}

void GameRunner::SetupCombatCamera(const BAK::Encounter::Encounter&)
{
    mSavedCameraAngle = mCamera.GetAngle();
    mSavedCameraPos = mCamera.GetPosition();

    // Snap the camera to the nearest 4-bak angle of the heading
    auto heading = mCamera.GetGameAngle();//BAK::SnapHeading(mCamera.GetGameAngle());
    auto angle = mCamera.GetAngle();

    angle.x = BAK::ToGlAngle(heading).x;
    angle.y = BAK::gBakCombatCameraDownAngle;
    mCamera.SetAngle(angle);

    auto pos = mCamera.GetPosition();
    pos.y = BAK::gBakCombatCameraHeight;
    mCamera.SetPosition(pos);
}

void GameRunner::RestoreCameraAfterCombat()
{
    HideGrid();

    mCamera.SetAngle(mSavedCameraAngle);
    mCamera.SetPosition(mSavedCameraPos);
}

void GameRunner::CombatCompleted(BAK::CombatResult result)
{
    auto onReturn = ScopeGuard{[this]{
        mGuiManager.SetCombatSequenceActive(false);
        RestoreCameraAfterCombat();
        ClearCombatActors();
        LoadWorldActors();
    }};

    mLogger.Debug() << __FUNCTION__ << " " << ToString(result) << "\n";
    ASSERT(mActiveEncounter);
    ASSERT(std::holds_alternative<BAK::Encounter::Combat>(mActiveEncounter->GetEncounter()));
    const auto& encounter = *mActiveEncounter;
    const auto& combat = std::get<BAK::Encounter::Combat>(encounter.GetEncounter());

    if (result == BAK::CombatResult::Fled)
    {
        const auto& retreatPos = BAK::Encounter::GetRetreatPosition(
            combat, mRetreatDirection);

        mSavedCameraPos = BAK::ToGlCoord<float>(retreatPos.mPosition);
        mSavedCameraPos.y = BAK::gBakCameraHeight;
        mSavedCameraAngle = BAK::ToGlAngle(retreatPos.mHeading);

        mGameState.SetLocation(
            BAK::Location{
                mGameState.GetZone(),
                BAK::GetTile(retreatPos.mPosition),
                retreatPos});

        mEncounterHandler.StartDialog(BAK::DialogSources::mRetreatSuccessful, true);
        return;
    }
    else if (result == BAK::CombatResult::Won)
    {
        mEncounterHandler.GetCombatHandler().UpdatePostEncounterFlags(encounter, combat);

        unsigned i = 0;
        for (auto cIdx: mGameState.GetCombatEntityList(combat.mCombatIndex).mCombatants)
        {
            auto& cgl = mGameState.GetCombatantGridLocation(cIdx);
            cgl.mState |= std::to_underlying(BAK::Combat::CombatantState::Dead);

            auto& cwl = mGameState.GetCombatWorldLocation(
                encounter.GetTileIndex(), encounter.GetTileCombatIndex(), i);
            cwl.mState = BAK::CombatantWorldState::Dead;

            auto combatantPos = BAK::MakeGamePositionFromGridCell(
                mGameState.GetLocation(), cgl.mGridPos);
            cwl.mPosition.mPosition = BAK::GetTileSpaceOffset(combatantPos);
            i += 1;
        }

        if (combat.mCombatIndex == BAK::MakalaCombat)
        {
            mEncounterHandler.StartDialog(BAK::DialogSources::mDefeatedMakala, true);
            return;
        }

        unsigned deadCount = 0;

        mGameState.GetParty().ForEachActiveCharacter(
            [&](const auto& character){
                if (character.GetConditions()
                    .GetCondition(BAK::Condition::NearDeath).Get() > 0)
                {
                    mGameState.SetActiveCharacter(character.GetIndex());
                    deadCount++;
                }
                return BAK::Loop::Continue;
        });


        if (deadCount > 0)
        {
            mGameState.GetPartyChangeCache().ClearCondition(mGameState, BAK::Condition::NearDeath);
            mEncounterHandler.StartDialog(BAK::DialogSources::mSomeoneDied, true);
        }
        else
        {
            if (true) // multiple combatants killed
            {
                //if (MonstersWereGhosts)
                //{
                //    mEncounterHandler.StartDialog(BAK::DialogSources::mWonVersusGhosts, true);
                //}
                if (BAK::IsSpecialBattle(combat.mCombatIndex))
                {
                    mEncounterHandler.StartDialog(BAK::DialogSources::mWonSpecialBattle, true);
                }
                else
                {
                    mEncounterHandler.StartDialog(BAK::DialogSources::mWonBattle, true);
                }
            }
            else // if (OneMonsterInCombat && IsGhost)
            {
                mEncounterHandler.StartDialog(BAK::DialogSources::mWonVersusGhost, true);
            }
            // else if (OneMonsterInCombat && !IsGhost)
            // mEncounterHandler.StartDialog(BAK::DialogSources::mDefeatedOneEnemy, true);
            // else if (NoCombatantsRemaining && IsTrap)
            // mEncounterHandler.StartDialog(BAK::DialogSources::mSolvedTrap, true);
            // else if (NoCombatantsRemaining)
            // mEncounterHandler.StartDialog(BAK::DialogSources::mEnemyFled, true);
        }
    }
}

void GameRunner::EnterCombatFromEncounter()
{
    ASSERT(mActiveEncounter);
    ASSERT(std::holds_alternative<BAK::Encounter::Combat>(mActiveEncounter->GetEncounter()));
    const auto& encounter = *mActiveEncounter;
    const auto& combat = std::get<BAK::Encounter::Combat>(encounter.GetEncounter());

    UnloadWorldActors();

    auto playerPos = mGameState.GetLocation();
    mRetreatDirection = BAK::Encounter::CalculateRetreatDirection(
        encounter, playerPos.mPosition);
    SetupCombatCamera(encounter);

    if (mGridVisible)
        HideGrid();
    ShowGrid(playerPos);

    for (auto cIdx: mGameState.GetCombatEntityList(combat.mCombatIndex).mCombatants)
    {
        const auto& cgl = mGameState.GetCombatantGridLocation(cIdx);
        if (CheckBitSet(cgl.mState, BAK::Combat::CombatantState::Dead))
        {
            continue;
        }

        auto monsterIndex = BAK::MonsterIndex{cgl.mMonster.mValue};

        auto combatPos = BAK::MakeGamePositionFromGridCell(playerPos, cgl.mGridPos);
        auto entityId = mCombatActorStore.AddActor(
            combatPos, monsterIndex);
        auto& actor = *mCombatActorStore.GetActor(entityId);
        actor.Update();
        mSystems->AddDynamicRenderable(
            DynamicRenderable{
                entityId,
                &actor.GetRenderData(),
                &actor.mObject,
                &actor.mLocation,
                &actor.mModelMatrix});

        mCombatManager.AddCombatant(Combat::Combatant{
            nullptr,
            monsterIndex,
            cgl.mGridPos,
            BAK::Combat::CombatantState::Alive,
            entityId
        });
    }

    std::vector<glm::uvec2> charPos = {{0, 1}, {7, 1}, {7, 12}};
    unsigned i = 0;

    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        auto combatPos = BAK::MakeGamePositionFromGridCell(playerPos, charPos[i]);
        auto entityId = mCombatActorStore.AddActor(
            combatPos, character.GetMonsterIndex());
        auto& actor = *mCombatActorStore.GetActor(entityId);
        actor.SetState(BAK::AnimationType::Idle, BAK::Direction::North);
        actor.Update();
        mSystems->AddDynamicRenderable(
            DynamicRenderable{
                entityId,
                &actor.GetRenderData(),
                &actor.mObject,
                &actor.mLocation,
                &actor.mModelMatrix});

        mCombatManager.AddCombatant(Combat::Combatant{
            &character,
            character.GetMonsterIndex(),
            charPos[i],
            BAK::Combat::CombatantState::Alive,
            entityId
        });

        i++;
        return BAK::Loop::Continue;
    });

    auto [fastestCharacter, _] = mGameState.GetParty().GetSkill(BAK::SkillType::Speed, true);
    auto fastestActive = mGameState.GetParty().FindActiveCharacter(fastestCharacter);
    mGuiManager.EnterCombat([this](BAK::CombatResult result){
            CombatCompleted(result);
        },
        fastestActive);
}

bool GameRunner::CheckAndDoEncounter(glm::uvec2 position)
{
    mLogger.Debug() << __FUNCTION__ << " Pos: " << position << "\n";
    auto intersectables = mSystems->RunIntersection(
        BAK::ToGlCoord<float>(position));
    if (!intersectables.empty())
    {
        auto it = mEncounters.find(intersectables.back());
        if (it != mEncounters.end())
        {
            const auto* encounter = it->second;
            mActiveEncounter = encounter;
            if (mActiveEncounter)
            {
                return mEncounterHandler.DoEncounter(*mActiveEncounter);
            }
        }
    }
    return false;
}

void GameRunner::RunGameUpdate(bool advanceTime)
{
    if (mCamera.CheckAndResetDirty())
    {
        // only required for imgui, can remove at some point
        mActiveEncounter = nullptr;

        // Need to handle multiple intersectables.
        auto intersectables = mSystems->RunIntersection(mCamera.GetPosition());
        if (!intersectables.empty())
        {
            auto it = mEncounters.find(intersectables.back());
            if (it != mEncounters.end())
            {
                const auto* encounter = it->second;
                mActiveEncounter = encounter;
            }
        }

        if (auto unitsTravelled = mCamera.GetAndClearUnitsTravelled(); unitsTravelled > 0 && advanceTime)
        {
            auto camp = BAK::TimeChanger{ mGameState };
            camp.ElapseTimeInMainView(
                BAK::Time{0x1e * unitsTravelled});

            // 1. If within X units of a tile, load the combat encounters into the CombatWorldLocation
            //    if they haven't been already
        }

        if (mActiveEncounter)
        {
            mEncounterHandler.DoEncounter(*mActiveEncounter);
        }
    }
}

void GameRunner::CheckClickable(unsigned entityId)
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

void GameRunner::OnTimeDelta(double timeDelta)
{
    mAccumulatedTime += timeDelta;
    if (mAccumulatedTime > .5)
    {
        mAccumulatedTime = 0;
        for (auto& actor : mCombatActorStore.GetActors())
        {
            actor.mFrame += 1;
            actor.Update();
        }
    }
}

const Graphics::RenderData& GameRunner::GetZoneRenderData() const
{
    assert(mZoneRenderData);
    return *mZoneRenderData;
}

const BAK::Encounter::Encounter& GameRunner::FindEncounterByCombatIndex(BAK::CombatIndex combatIndex) const
{
    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        for (const auto& encounter : world.GetEncounters(mGameState.GetChapter()))
        {
            if (!std::holds_alternative<BAK::Encounter::Combat>(encounter.GetEncounter()))
                continue;
            const auto& combat = std::get<BAK::Encounter::Combat>(encounter.GetEncounter());
            if (combat.mCombatIndex == combatIndex)
                return encounter;
        }
    }
    ASSERT(false);
}

void GameRunner::ShowGrid(const BAK::GamePositionAndHeading& orientation)
{
    if (!mSystems || !mZoneData)
        return;

    auto gridRotation = BAK::ToGlAngle(orientation.mHeading).x;

    for (unsigned row = 0; row < BAK::gCombatGridRows; row++)
    {
        for (unsigned col = 0; col < BAK::gCombatGridCols; col++)
        {
            auto worldPos = BAK::MakeGamePositionFromGridCell(
                orientation, glm::uvec2{col, row});
            auto glPos = BAK::ToGlCoord<float>(worldPos)
                + glm::vec3{0, 1.0f, 0};

            auto id = mSystems->GetNextItemId();
            mGridCellRenderables.emplace_back(Renderable{
                id,
                mZoneData->mObjects.GetObject("GridCell"),
                glPos,
                glm::vec3{0, gridRotation, 0},
                glm::vec3{BAK::gCombatGridCellSize, 1, BAK::gCombatGridCellSize} / BAK::gWorldScale});
            mGridCellEntityIds.emplace_back(id);
            mSystems->AddRenderable(mGridCellRenderables.back());
        }
    }
    mGridVisible = true;
    mLogger.Debug() << "Grid shown\n";
}

void GameRunner::HideGrid()
{
    if (!mGridVisible)
        return;

    for (const auto& id : mGridCellEntityIds)
        mSystems->RemoveRenderable(id);
    mGridCellRenderables.clear();
    mGridCellEntityIds.clear();
    mGridVisible = false;
    mLogger.Debug() << "Grid hidden\n";
}

bool GameRunner::HandleGridCellClick(unsigned entityId)
{
    for (unsigned i = 0; i < mGridCellEntityIds.size(); i++)
    {
        if (mGridCellEntityIds[i].mValue == entityId)
        {
            const auto row = i / BAK::gCombatGridCols;
            const auto col = i % BAK::gCombatGridCols;
            mLogger.Info() << "Grid cell (" << col << ", " << row << ") clicked\n";

            auto charIndex = mGameState.GetParty()
                .GetCharacter(BAK::ActiveCharIndex{0}).GetIndex();
            auto* combatant = mCombatManager.GetCombatant(charIndex);
            if (!combatant)
            {
                mLogger.Error() << "Couldn't find char with ix: " << charIndex << " in combat\n";
                return true;
            }

            auto* actor = mCombatActorStore
                .GetActor(combatant->mEntityIndex);
            if (!actor)
            {
                mLogger.Error() << "Couldn't find actor with entityId: " << combatant->mEntityIndex << " in actor store\n";
                return true;
            }

            auto playerPos = mGameState.GetLocation();
            auto combatPos = BAK::MakeGamePositionFromGridCell(playerPos, glm::uvec2{col, row});
            actor->mLocation = BAK::ToGlCoord<float>(combatPos);
            mLogger.Info() << "Moving actor: " << actor->mMonster << " to: " << combatPos << "\n";
            actor->SetState(BAK::AnimationType::Thrust, BAK::Direction::South);
            actor->Update();
            return true;
        }
    }
    return false;
}

void GameRunner::CleanCombatsOnNewZone()
{
    UnloadWorldActors();

    for (auto& cwl : mGameState.GetCombatWorldLocations())
    {
        cwl.mPosition = BAK::GamePositionAndHeading{};
        cwl.mImageIndex = 0;
        cwl.mState = BAK::CombatantWorldState::Invisible1;
    }
}
}
