#include "game/gameRunner.hpp"

#include "game/combat/flashAnimator.hpp"
#include "game/combat/frameAnimator.hpp"
#include "game/combat/moveAnimator.hpp"
#include "game/combatModelLoader.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/combat/combatModel.hpp"
#include "bak/collision.hpp"
#include "bak/camera.hpp"
#include "bak/combat/mechanics.hpp"

#include "gui/colors.hpp"
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

#include "graphics/glm.hpp"

#include "gui/guiManager.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>
#include <unordered_set>
#include <utility>
#include <variant>

namespace Game {

GameRunner::GameRunner(
    Camera& camera,
    BAK::GameState& gameState,
    Gui::GuiManager& guiManager,
    bool debugRenderEncounters,
    double animationSpeedMultiplier)
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
    mCombatManager{*this, mGuiManager.GetCombatUI()},
    mClickablesEnabled{false},
    mDebugRenderEncounters{debugRenderEncounters},
    mAnimationSpeedMultiplier{animationSpeedMultiplier},
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

    mGlyphStore.Init(mGuiManager.GetFontManager().GetGameFont());
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
        mZoneData->mObjects,
        mZoneData->mZoneTextures.GetTextures(),
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
    mEntityTypes.clear();
    mActiveEncounter = nullptr;

    std::vector<glm::uvec2> handledLocations{};

    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        for (const auto& item : world.GetItems())
        {
            if (item.GetZoneItem().GetVertices().size() > 1)
            {
                auto id = mSystems->GetNextItemId();
                mEntityTypes[id] = item.GetZoneItem().GetEntityType();
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
        mEntityTypes[id] = item.GetEntityType();
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
        mEntityTypes[id] = item.GetEntityType();
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

    mClipRenderables.clear();
    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        for (const auto& item : world.GetItems())
        {
            if (item.GetZoneItem().GetModelClip()
                && item.GetZoneItem().GetVertices().size() > 1)
            {
                auto id = mSystems->GetNextItemId();
                auto renderable = Renderable{
                    id,
                    mZoneData->mObjects.GetObject(
                        "clip_" + item.GetZoneItem().GetName()),
                    item.GetLocation(),
                    item.GetRotation(),
                    glm::vec3{
                        static_cast<float>(item.GetZoneItem().GetScale())}};
                renderable.SetVisible(mClipDisplayMode != ClipDisplayMode::Vanilla);
                mClipRenderables.emplace_back(std::move(renderable));
            }
        }
    }

    mGridVisible = false;
    mGridCellRenderables.clear();
    mGridCells.clear();
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
                actor.SetState(BAK::AnimationType::Dead, BAK::Direction::South);
            mSystems->AddDynamicRenderable(
                DynamicRenderable{
                    entityId,
                    &actor.GetRenderData(),
                    &actor.mObject,
                    &actor.mLocation,
                    &actor.mModelMatrix});

            auto* container = mGameState.GetCombatContainer(BAK::CombatRelInfo{combat.mCombatIndex, i});
            if (container)
            {
                const auto entityType = combatComplete
                    ? BAK::EntityType::DEAD_COMBATANT
                    : BAK::EntityType::LIVING_COMBATANT;
                mSystems->AddClickable(Clickable{entityId});
                mClickables.emplace(entityId, ClickableEntity(entityType, container));
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
    mSystems->ClearTextRenderables();
    mCombatActorStore.Clear();
    mCombatManager.EndCombat();
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

    auto heading = mCamera.GetGameAngle();
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

void GameRunner::CombatFinished(BAK::CombatResult result)
{
    // Yuck... we probably don't need to go in and then back
    // here to exit combat...
    mGuiManager.ExitCombat(result);
}

void GameRunner::CombatCompleted(BAK::CombatResult result)
{
    auto onReturn = ScopeGuard{[this]{
        mGuiManager.SetCombatSequenceActive(false);
        RestoreCameraAfterCombat();
        ClearCombatActors();
        LoadWorldActors();
    }};

    mLogger.Debug() << __FUNCTION__ << " " << ToString(result.mOutcome) << "\n";
    ASSERT(mActiveEncounter);
    ASSERT(std::holds_alternative<BAK::Encounter::Combat>(mActiveEncounter->GetEncounter()));
    const auto& encounter = *mActiveEncounter;
    const auto& combat = std::get<BAK::Encounter::Combat>(encounter.GetEncounter());

    if (result.mOutcome == BAK::CombatOutcome::Fled)
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
    else if (result.mOutcome == BAK::CombatOutcome::Won)
    {
        mEncounterHandler.GetCombatHandler().UpdatePostEncounterFlags(encounter, combat);

        unsigned i = 0;
        for (auto combatantIndex: mGameState.GetCombatEntityList(combat.mCombatIndex).mCombatants)
        {
            auto& cgl = mGameState.GetCombatantGridLocation(combatantIndex);
            cgl.mState |= std::to_underlying(BAK::Combat::CombatantState::Dead);

            auto& cwl = mGameState.GetCombatWorldLocation(
                encounter.GetTileIndex(), encounter.GetTileCombatIndex(), i);
            cwl.mState = BAK::CombatantWorldState::Dead;

            auto combatantPos = BAK::MakeGamePositionFromGridCell(
                mCombatPlayerPos, cgl.mGridPos);
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
            mEncounterHandler.StartDialog(result.mDialog, true);
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

    mCombatPlayerPos = mGameState.GetLocation();
    mRetreatDirection = BAK::Encounter::CalculateRetreatDirection(
        encounter, mCombatPlayerPos.mPosition);
    SetupCombatCamera(encounter);

    for (auto combatantIndex : mGameState.GetCombatEntityList(combat.mCombatIndex).mCombatants)
    {
        const auto& cgl = mGameState.GetCombatantGridLocation(combatantIndex);
        if (CheckBitSet(cgl.mState, BAK::Combat::CombatantState::Dead))
        {
            continue;
        }

        auto monsterIndex = BAK::MonsterIndex{cgl.mMonster.mValue};

        auto combatPos = BAK::MakeGamePositionFromGridCell(mCombatPlayerPos, cgl.mGridPos);
        auto entityId = mCombatActorStore.AddActor(
            combatPos, monsterIndex);
        auto& actor = *mCombatActorStore.GetActor(entityId);
        actor.RandomiseIdleFrame();
        mSystems->AddDynamicRenderable(
            DynamicRenderable{
                entityId,
                &actor.GetRenderData(),
                &actor.mObject,
                &actor.mLocation,
                &actor.mModelMatrix,
                &actor.mFlashColor});

        mCombatManager.AddCombatant(Combat::Combatant{
            mGameState.GetCombatantCharacter(combatantIndex),
            monsterIndex,
            cgl.mGridPos,
            BAK::Combat::CombatantState::Alive,
            entityId
        });
    }

    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        auto gridPos = character.GetGridPos();
        auto combatPos = BAK::MakeGamePositionFromGridCell(mCombatPlayerPos, gridPos);
        auto entityId = mCombatActorStore.AddActor(
            combatPos, character.GetMonsterIndex());
        auto& actor = *mCombatActorStore.GetActor(entityId);
        actor.SetState(BAK::AnimationType::Idle, BAK::Direction::North);
        actor.RandomiseIdleFrame();
        mSystems->AddDynamicRenderable(
            DynamicRenderable{
                entityId,
                &actor.GetRenderData(),
                &actor.mObject,
                &actor.mLocation,
                &actor.mModelMatrix,
                &actor.mFlashColor});

        mCombatManager.AddCombatant(Combat::Combatant{
            &character,
            character.GetMonsterIndex(),
            gridPos,
            BAK::Combat::CombatantState::Alive,
            entityId
        });
        mLogger.Debug() << "Added character entity: " << entityId << " " << character.GetName() << "\n";
        return BAK::Loop::Continue;
    });

    mCombatManager.BeginCombat();

    if (mGridVisible)
    {
        HideGrid();
    }
    ShowGrid();

    UpdateGridCellColors();

    mGuiManager.EnterCombat([this](BAK::CombatResult result){
            CombatCompleted(result);
        });
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

bool GameRunner::CannotMoveHere(BAK::GamePosition playerPos) const
{
    if (!mZoneData) return false;

    const auto p = glm::ivec2{playerPos};

    for (const auto& world : mZoneData->mWorldTiles.GetTiles())
    {
        for (const auto& item : world.GetItems())
        {
            const auto& zi = item.GetZoneItem();
            const auto& modelClip = zi.GetModelClip();
            if (!modelClip)
            {
                continue;
            }

            if (BAK::GetClipEffect(zi.GetEntityType()) != BAK::ClipEffect::Block)
            {
                continue;
            }

            auto local = glm::vec2{p - glm::ivec2{item.GetBakLocation()}};
            local /= zi.GetScale();

            const auto angle = item.GetRotation().y;
            glm::vec2 rot = local;
            if (angle != 0.0f)
            {
                rot = glm::vec2{
                    glm::rotate(glm::vec3{local, 0}, -angle, glm::vec3{0, 0, 1})};
            }

            if (BAK::PointInModelClip(rot, *modelClip))
            {
                return true;
            }
        }
    }

    return false;
}

void GameRunner::RunGameUpdate(bool advanceTime)
{
    if (mCamera.CheckAndResetDirty())
    {
        if (CannotMoveHere(mCamera.GetGameLocation().mPosition))
        {
            mLogger.Debug() << "Move rejected by collision\n";
            mCamera.UndoPositionChange();
            return;
        }

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

void GameRunner::SetHoveredEntity(std::optional<BAK::EntityIndex> entityId)
{
    if (entityId == mHoveredEntity || mAnimationActive)
    {
        return;
    }

    mHoveredEntity = entityId;
    if (mCombatManager.IsCombatActive() && entityId)
    {
        for (unsigned i = 0; i < mGridCells.size(); i++)
        {
            if (mGridCells[i].mEntityId == *entityId)
            {
                mCombatManager.OnHoverChanged(mGridCells[i].mGridPos);
                UpdateGridCellColors();
                return;
            }
        }
    }
    else if (mGridVisible)
    {
        mCombatManager.OnHoverChanged(std::nullopt);
        UpdateGridCellColors();
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
    for (auto& actor : mCombatActorStore.GetActors())
    {
        actor.AdvanceIdle(timeDelta);
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

void GameRunner::ShowGrid()
{
    if (!mSystems || !mZoneData)
        return;

    auto gridRotation = BAK::ToGlAngle(mCombatPlayerPos.mHeading).x;
    auto& grid = mCombatManager.GetGrid();

    mGridCells.resize(BAK::gCombatGridRows * BAK::gCombatGridCols);

    auto gridMin = glm::vec2{};
    auto gridMax = glm::vec2{};
    auto gridCellIds = std::unordered_set<BAK::EntityIndex>{};

    for (unsigned row = 0; row < BAK::gCombatGridRows; row++)
    {
        for (unsigned col = 0; col < BAK::gCombatGridCols; col++)
        {
            auto worldPos = BAK::MakeGamePositionFromGridCell(
                mCombatPlayerPos, glm::uvec2{col, row});
            auto glPos = BAK::ToGlCoord<float>(worldPos)
                + glm::vec3{0, 1.0f, 0};

            gridMin.x = std::min(gridMin.x, glPos.x);
            gridMax.x = std::max(gridMax.x, glPos.x);
            gridMin.y = std::min(gridMin.y, glPos.z);
            gridMax.y = std::max(gridMax.y, glPos.z);

            auto id = mSystems->GetNextItemId();
            auto i = mGridCellRenderables.size();
            mGridCells[i] = GridCellInfo{
                std::nullopt,
                id,
                Game::Combat::GridPos{
                    static_cast<int>(col),
                    static_cast<int>(row)}};
            gridCellIds.insert(id);
            mGridCellRenderables.emplace_back(Renderable{
                id,
                mZoneData->mObjects.GetObject("GridCell"),
                glPos,
                glm::vec3{0, gridRotation, 0},
                glm::vec3{BAK::gCombatGridCellSize - 1, 1, BAK::gCombatGridCellSize - 1} / BAK::gWorldScale,
                &mGridCells[i].mColor});
            mSystems->AddRenderable(mGridCellRenderables.back());
        }
    }
    mGridVisible = true;

    auto halfCell = BAK::gCombatGridCellSize / 2.0f;
    gridMin -= halfCell;
    gridMax += halfCell;

    auto isInBounds = [&](const glm::vec3& loc)
    {
        return !(loc.x < gridMin.x || loc.x > gridMax.x
              || loc.z < gridMin.y || loc.z > gridMax.y);
    };

    auto processByEffect = [&](BAK::GridEffect desired, auto&& handler)
    {
        auto process = [&](const auto& entities)
        {
            for (const auto& entity : entities)
            {
                auto it = mEntityTypes.find(entity.GetId());
                if (it == mEntityTypes.end())
                {
                    continue;
                }

                auto effect = BAK::GetGridEffect(it->second);
                if (effect != desired || !isInBounds(entity.GetLocation()))
                {
                    continue;
                }
                handler(entity.GetId(), entity.GetLocation());
            }
        };
        process(mSystems->GetRenderables());
        process(mSystems->GetSprites());
    };

    processByEffect(BAK::GridEffect::Hidden,
        [&](auto id, const auto&)
        {
            mSystems->EnableRenderable(id, false);
            mSystems->EnableSprite(id, false);
            mHiddenWorldItems.emplace_back(id);
        });

    std::unordered_set<unsigned> disabledCellIndices;

    processByEffect(BAK::GridEffect::NotWalkable,
        [&](auto id, const auto& loc)
        {
            auto bestDistance = halfCell;
            std::optional<unsigned> bestIndex;
            for (unsigned i = 0; i < mGridCellRenderables.size(); i++)
            {
                const auto& cellLoc = mGridCellRenderables[i].GetLocation();
                auto distance = glm::distance(
                    glm::vec2{loc.x, loc.z},
                    glm::vec2{cellLoc.x, cellLoc.z});
                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestIndex = i;
                }
            }
            if (bestIndex)
            {
                disabledCellIndices.insert(*bestIndex);
            }
        });

    if (!disabledCellIndices.empty())
    {
        std::vector<Combat::GridPos> disabledCells;
        disabledCells.reserve(disabledCellIndices.size());
        for (auto idx : disabledCellIndices)
        {
            disabledCells.emplace_back(mGridCells[idx].mGridPos);
        }
        mCombatManager.DisableCells(disabledCells);
    }
}

void GameRunner::ToggleDisplayAllCells()
{
    mCombatManager.ToggleDisplayAllCells();
    if (mGridVisible)
        UpdateGridCellColors();
}

void GameRunner::UpdateGridCellColors()
{
    for (unsigned i = 0; i < mGridCells.size(); i++)
    {
        auto color = mCombatManager.GetGridCellColor(
            mGridCells[i].mGridPos.x, mGridCells[i].mGridPos.y);
        mGridCells[i].mColor = color.a > 0
            ? std::optional<glm::vec4>{color}
            : std::nullopt;
        mSystems->EnableRenderable(mGridCells[i].mEntityId, color.a > 0);
    }
}

void GameRunner::HideGrid()
{
    if (!mGridVisible)
        return;

    for (auto id : mHiddenWorldItems)
    {
        mSystems->EnableRenderable(id, true);
        mSystems->EnableSprite(id, true);
    }
    mHiddenWorldItems.clear();

    for (const auto& cell : mGridCells)
        mSystems->RemoveRenderable(cell.mEntityId);
    mGridCellRenderables.clear();
    mGridCells.clear();
    mGridVisible = false;
    mLogger.Debug() << "Grid hidden\n";
}

void GameRunner::MoveCombatant(
    BAK::EntityIndex entityId,
    glm::uvec2 sourceGrid,
    glm::uvec2 targetGrid)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    auto bakPos = BAK::MakeGamePositionFromGridCell(mCombatPlayerPos, targetGrid);
    auto targetPos = BAK::ToGlCoord<float>(bakPos);
    mLogger.Debug() << "Moving combatant: " << entityId
        << " cam: " << mCombatPlayerPos << " pos: " << bakPos << "\n";

    auto startPos = actor->mLocation;

    auto direction = BAK::GetDirectionBetween(sourceGrid, targetGrid);
    actor->SetDirection(direction);

    auto moveDuration = sMoveDuration * mAnimationSpeedMultiplier;

    mAnimationActive = true;
    UpdateGridCellColors();
    mGuiManager.AddAnimator(
        std::make_unique<Combat::MoveAnimator>(
            *actor,
            startPos,
            targetPos,
            moveDuration,
            [this, targetGrid]() mutable {
                mAnimationActive = false;
                mCombatManager.CompleteMove(
                    Combat::GridPos(targetGrid));
                UpdateGridCellColors();
            }));
}

void GameRunner::SetCombatantAction(
    BAK::EntityIndex entityId,
    BAK::AnimationType animType)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    mLogger.Spam() << "Setting combatant action: " << entityId
        << " mid: " << actor->mMonster 
        << " anim: " << ToString(animType) << "\n";
    actor->StartAnimation(animType);
}

void GameRunner::SetCombatantDirection(
    BAK::EntityIndex entityId,
    BAK::Direction direction)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    mLogger.Spam() << "Setting combatant direction: " << entityId
        << " dir: " << static_cast<unsigned>(direction) << "\n";
    actor->SetDirection(direction);
}

void GameRunner::SetCombatantUpdateIdle(
    BAK::EntityIndex entityId,
    bool update)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    actor->SetUpdateIdle(false);
}

void GameRunner::AnimateCombatant(
    BAK::EntityIndex entityId)
{
    AnimateCombatant(entityId, []{});
}

void GameRunner::AnimateCombatant(
    BAK::EntityIndex entityId,
    std::function<void()> onFinished)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    mLogger.Debug() << "Animating combatant (with callback): " << entityId 
        << " mid: " << actor->mMonster << "\n";

    assert(BAK::IsCardinal(actor->mDirection));

    auto frameTime = sFrameTime * mAnimationSpeedMultiplier;

    mAnimationActive = true;
    mGuiManager.AddAnimator(
        std::make_unique<Combat::FrameAnimator>(
            *actor,
            frameTime,
            [this, finished=std::move(onFinished)]() mutable {
                mAnimationActive = false;
                finished();
                UpdateGridCellColors();
            }));
}

void GameRunner::AnimateAttack(
    BAK::EntityIndex entityId,
    glm::uvec2 targetGrid)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    mLogger.Debug() << "Animating attack: " << entityId 
        << " mid: " << actor->mMonster << "\n";
    assert(BAK::IsCardinal(actor->mDirection));

    auto frameTime = sFrameTime * mAnimationSpeedMultiplier;

    mAnimationActive = true;
    mGuiManager.AddAnimator(
        std::make_unique<Combat::FrameAnimator>(
            *actor,
            frameTime,
            [this, targetGrid]() mutable {
                mAnimationActive = false;
                mCombatManager.CompleteAttack(
                    Combat::GridPos(targetGrid));
                UpdateGridCellColors();
            }));
}

bool GameRunner::HandleGridCellClick(unsigned entityId, bool isRightClick)
{
    if (mAnimationActive)
    {
        return true;
    }

    for (unsigned i = 0; i < mGridCells.size(); i++)
    {
        if (mGridCells[i].mEntityId.mValue == entityId)
        {
            mLogger.Info() << "Grid cell (" << mGridCells[i].mGridPos.x << ", " << mGridCells[i].mGridPos.y << ") "
                << (isRightClick ? "right-" : "") << "clicked\n";

            mCombatManager.GridCellClicked(glm::uvec2{mGridCells[i].mGridPos}, isRightClick);
            return true;
        }
    }
    return false;
}

void GameRunner::DisplayText(
    BAK::EntityIndex target,
    std::string text,
    TextColor color)
{
    auto* actor = mCombatActorStore.GetActor(target);

    auto worldPos = actor->mLocation;
    worldPos.y += sDamageTextHeightOffset;

    mGuiManager.AddAnimator(
        std::make_unique<TextAnimator>(
            *mSystems,
            mGlyphStore,
            mCamera,
            worldPos,
            text,
            color,
            sHitFlashDuration));
}

void GameRunner::FlashCombatant(BAK::EntityIndex entityId, glm::vec4 color)
{
    auto* actor = mCombatActorStore.GetActor(entityId);
    assert(actor);
    mGuiManager.AddAnimator(
        std::make_unique<Combat::FlashAnimator>(*actor, color));
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

void GameRunner::SetClipDisplayMode(ClipDisplayMode mode)
{
    mClipDisplayMode = mode;
    for (auto& r : mClipRenderables)
    {
        r.SetVisible(mClipDisplayMode != ClipDisplayMode::Vanilla);
    }
}
}
