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
    mCombatantManager{
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
    mCombatantManager.SetSystems(mSystems.get());
    mCombatantManager.clear();
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

        LoadTileVisibleCombatants(world.GetTileIndex());
    }

    mGridVisible = false;
    mGridCellRenderables.clear();
    mGridCellEntityIds.clear();
}

void GameRunner::LoadTileVisibleCombatants(std::uint8_t tileIndex)
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


            auto entityId = mCombatantManager.AddCombatant(worldPos, BAK::MonsterIndex{combatant.mMonster});
            entityIndices.push_back(entityId);
            auto& activeCombatant = *mCombatantManager.GetActiveCombatant(entityId);
            activeCombatant.Update();
            mSystems->AddDynamicRenderable(
                DynamicRenderable{
                    entityId,
                    &activeCombatant.GetRenderData(),
                    &activeCombatant.mObject,
                    &activeCombatant.mLocation,
                    &activeCombatant.mRotation,
                    &activeCombatant.mScale});

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

        mCombatsToActiveCombatants.emplace(combat.mCombatIndex.mValue, std::move(entityIndices));
    }
}

void GameRunner::DoGenericContainer(BAK::EntityType et, BAK::GenericContainer& container)
{
    mLogger.Debug() << __FUNCTION__ << " " 
        << static_cast<unsigned>(et) << " " << container << "\n";

    mCurrentInteractable = mInteractableFactory.MakeInteractable(et);
    ASSERT(mCurrentInteractable);
    mCurrentInteractable->BeginInteraction(container, et);
}

void GameRunner::SetupCombatCamera()
{
    mSavedCameraAngle = mCamera.GetAngle();
    mSavedCameraPos = mCamera.GetPosition();

    auto angle = mCamera.GetAngle();

    auto heading = mCamera.GetGameAngle();
    switch (BAK::HeadingToCardinalDirection(heading))
    {
    case BAK::CardinalDirection::North: heading = 0; break;
    case BAK::CardinalDirection::West:  heading = 64; break;
    case BAK::CardinalDirection::South: heading = 128; break;
    case BAK::CardinalDirection::East:  heading = 192; break;
    }
    angle.x = BAK::ToGlAngle(heading).x;

    angle.y = glm::radians(-15.0f);
    mCamera.SetAngle(angle);

    auto pos = mCamera.GetPosition();
    pos.y = 900.0f;
    mCamera.SetPosition(pos);
}

void GameRunner::RestoreCombatCamera()
{
    if (mGridVisible)
        ToggleGrid();

    mCamera.SetAngle(mSavedCameraAngle);
    mCamera.SetPosition(mSavedCameraPos);
}

void GameRunner::CombatCompleted(BAK::CombatResult result)
{
    auto guard = ScopeGuard{[this]{ RestoreCombatCamera(); }};
    mLogger.Debug() << __FUNCTION__ << " " << ToString(result) << "\n";
    ASSERT(mActiveEncounter);
    ASSERT(std::holds_alternative<BAK::Encounter::Combat>(mActiveEncounter->GetEncounter()));
    const auto& encounter = *mActiveEncounter;
    const auto& combat = std::get<BAK::Encounter::Combat>(encounter.GetEncounter());

    if (result == BAK::CombatResult::Fled)
    {
        // retreat to a combat retreat location based on player entry
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

        const auto& entities = mCombatsToActiveCombatants.at(combat.mCombatIndex);
        for (auto entityId : entities)
        {
            auto it = mCombatantManager.GetActiveCombatant(entityId);
            if (!it)
            {
                continue;
            }

            const auto& monster = *mCombatModelLoader.mCombatModels[it->mMonster.mValue];
            const auto deadFrameOffset = monster.GetAnimation(
                BAK::AnimationType::Dead, BAK::Direction::South).mImageIndices.size() - 1;
            it->mAnimationType = BAK::AnimationType::Dead;
            it->mFrame = deadFrameOffset;
            it->Update();
            mClickables.at(entityId).mEntityType = BAK::EntityType::DEAD_COMBATANT;
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

    mCombatManager.Clear();

    if (mGridVisible)
    {
        ToggleGrid();
    }
    ToggleGrid();

    const auto& playerPos = mGameState.GetLocation();

    for (auto cIdx: mGameState.GetCombatEntityList(combat.mCombatIndex).mCombatants)
    {
        const auto& cgl = mGameState.GetCombatantGridLocation(cIdx);
        if (CheckBitSet(cgl.mState, BAK::Combat::CombatantState::Dead))
        {
            continue;
        }

        auto combatPos = BAK::MakeGamePositionFromGridCell(playerPos, cgl.mGridPos);
        auto entityId = mCombatantManager.AddCombatant(
            combatPos, BAK::MonsterIndex{cgl.mMonster});
        //entityIndices.push_back(entityId);
        auto& activeCombatant = *mCombatantManager.GetActiveCombatant(entityId);
        activeCombatant.Update();
        mSystems->AddDynamicRenderable(
            DynamicRenderable{
                entityId,
                &activeCombatant.GetRenderData(),
                &activeCombatant.mObject,
                &activeCombatant.mLocation,
                &activeCombatant.mRotation,
                &activeCombatant.mScale});


        //mCombatManager.AddCombatant({
        //    nullptr,
        //    cgl.mMonster,
        //    cgl.mGridPos,
        //    // we probably just want to pass the whole cgl in tbh...
        //    BAK::Combat::CombatantState::Alive});
    }

    std::vector<glm::uvec2> charPos = {{0, 1}, {7, 1}, {7, 12}};
    unsigned i = 0;

    mGameState.GetParty().ForEachActiveCharacter([&](auto& character)
    {
        auto combatPos = BAK::MakeGamePositionFromGridCell(playerPos, charPos[i++]);
        auto entityId = mCombatantManager.AddCombatant(
            combatPos, BAK::MonsterIndex{character.GetIndex().mValue + 15});
        //entityIndices.push_back(entityId);
        auto& activeCombatant = *mCombatantManager.GetActiveCombatant(entityId);
        activeCombatant.Update();
        mSystems->AddDynamicRenderable(
            DynamicRenderable{
                entityId,
                &activeCombatant.GetRenderData(),
                &activeCombatant.mObject,
                &activeCombatant.mLocation,
                &activeCombatant.mRotation,
                &activeCombatant.mScale});
        return BAK::Loop::Continue;
    });

    SetupCombatCamera();

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
    return;
    mAccumulatedTime += timeDelta;
    if (mAccumulatedTime > .2)
    {
        mAccumulatedTime = 0;
        /*for (auto& combatant : mActiveCombatants)
        {
            combatant.mFrame += 1;
            combatant.Update();
            const auto& model = mCombatModelLoader.mCombatModels[combatant.mMonster.mValue];
            if (combatant.mFrame == 0)
            {
                auto type = static_cast<BAK::AnimationType>((std::to_underlying(combatant.mAnimationType) + 1) % 9);
                const auto& anims = model->GetSupportedAnimations();
                if (std::find(anims.begin(), anims.end(), type) == anims.end())
                {
                    type = BAK::AnimationType::Idle;
                }
                if (type == BAK::AnimationType::Idle)
                {
                    if (combatant.mDirection == BAK::Direction::South)
                        combatant.mDirection = BAK::Direction::East;
                    else if (combatant.mDirection == BAK::Direction::East)
                        combatant.mDirection = BAK::Direction::North;
                    else if (combatant.mDirection == BAK::Direction::North)
                        combatant.mDirection = BAK::Direction::South;

                }
                combatant.mAnimationType = type;
                combatant.Update();
            }
        }*/
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

void GameRunner::ToggleGrid()
{
    if (!mSystems || !mZoneData)
        return;

    if (mGridVisible)
    {
        for (const auto& id : mGridCellEntityIds)
            mSystems->RemoveRenderable(id);
        mGridCellRenderables.clear();
        mGridCellEntityIds.clear();
        mGridVisible = false;
        mLogger.Debug() << "Grid hidden\n";
    }
    else
    {
        const auto& location = mGameState.GetLocation();
        for (unsigned row = 0; row < sGridRows; row++)
        {
            for (unsigned col = 0; col < sGridCols; col++)
            {
                auto worldPos = BAK::MakeGamePositionFromGridCell(
                    location, glm::uvec2{col, row});
                auto glPos = BAK::ToGlCoord<float>(worldPos)
                    + glm::vec3{0, 1.0f, 0};

                auto id = mSystems->GetNextItemId();
                mGridCellRenderables.emplace_back(Renderable{
                    id,
                    mZoneData->mObjects.GetObject("GridCell"),
                    glPos,
                    glm::vec3{0},
                    glm::vec3{sGridCellSize, 1, sGridCellSize} / BAK::gWorldScale});
                mGridCellEntityIds.emplace_back(id);
                mSystems->AddRenderable(mGridCellRenderables.back());
            }
        }
        mGridVisible = true;
        mLogger.Debug() << "Grid shown\n";
    }
}

bool GameRunner::HandleGridCellClick(unsigned entityId)
{
    for (unsigned i = 0; i < mGridCellEntityIds.size(); i++)
    {
        if (mGridCellEntityIds[i].mValue == entityId)
        {
            const auto row = i / sGridCols;
            const auto col = i % sGridCols;
            mLogger.Info() << "Grid cell [" << row << "][" << col << "] clicked\n";
            return true;
        }
    }
    return false;
}

void GameRunner::CleanCombatsOnNewZone()
{
    RestoreCombatCamera();
    //    mClickables.erase(combatant.mItemId);
    mCombatantManager.clear();
    mCombatsToActiveCombatants.clear();

    for (auto& cwl : mGameState.GetCombatWorldLocations())
    {
        cwl.mPosition = BAK::GamePositionAndHeading{};
        cwl.mImageIndex = 0;
        cwl.mState = BAK::CombatantWorldState::Invisible1;
    }
}
}
