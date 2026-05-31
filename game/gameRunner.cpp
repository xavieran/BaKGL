#include "game/gameRunner.hpp"

#include "bak/combatModel.hpp"
#include "bak/entityType.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/camera.hpp"
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
#include "com/ostream.hpp"

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
    mSystems{nullptr},
    mTeleportFactory{},
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
    mActiveCombatants.reserve(2056);
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

        LoadCombatants(world.GetTileIndex());
    }
}

void GameRunner::LoadCombatants(std::uint8_t tileIndex)
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
            auto& cwl = mGameState.GetCombatWorldLocation(tileIndex, encounter.mIndex.mValue, i);
            // Combatants that aren't dead in a completed combat should not be shown
            if (combatComplete
                && cwl.mState != BAK::CombatantWorldState::Dead)
            {
                continue;
            }
            const auto& combatant = combat.mCombatants[i];
            if (!mCombatModelLoader.mCombatModelDatas[combatant.mMonster])
            {
                mLogger.Error() << "Couldn't load combat model: " << combatant.mMonster << "\n";
                continue;
            }

            cwl.mImageIndex = 0;
            cwl.mState = static_cast<BAK::CombatantWorldState>(combatant.mMovementType);
            cwl.mPosition = combatant.mLocation;

            auto entityId = mSystems->GetNextItemId();
            entityIndices.emplace_back(entityId);
            const auto& datas = *mCombatModelLoader.mCombatModelDatas[combatant.mMonster];
            const auto& monster = *mCombatModelLoader.mCombatModels[combatant.mMonster];
            const auto deadFrameOffset = monster.GetAnimation(BAK::AnimationType::Dead, BAK::Direction::South).mImageIndices.size() - 1;

            mActiveCombatants.emplace_back(
                ActiveCombatant{
                    entityId,
                    {},
                    BAK::ToGlCoord<float>(combatant.mLocation.mPosition + tilePos),
                    Graphics::sNinetyDegreeRotation,
                    glm::vec3{1},
                    BAK::MonsterIndex{combatant.mMonster},
                    combatComplete ? BAK::AnimationType::Dead : BAK::AnimationType::Idle,
                    combatComplete ? BAK::Direction::North : BAK::Direction::South,
                    combatComplete ? deadFrameOffset : 3,
                    mCombatModelLoader,
                    cwl});

            auto& activeCombatant = mActiveCombatants.back();
            activeCombatant.Update();
            mSystems->AddDynamicRenderable(
                DynamicRenderable{
                    entityId,
                    &datas.mRenderData,
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

        mCombatsToActiveCombatants.emplace(combat.mCombatIndex, entityIndices);
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

void GameRunner::CombatCompleted(BAK::CombatResult result)
{
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

        const auto& entities = mCombatsToActiveCombatants.at(BAK::CombatIndex{combat.mCombatIndex});
        for (auto entityId : entities)
        {
            auto it = std::find_if(
                mActiveCombatants.begin(),
                mActiveCombatants.end(),
                [&entityId](const auto& combatant) {
                    return entityId == combatant.mItemId;
                });

            if (it == mActiveCombatants.end())
            {
                continue;
            }

            const auto& monster = *mCombatModelLoader.mCombatModels[it->mMonster.mValue];
            const auto deadFrameOffset = monster.GetAnimation(
                BAK::AnimationType::Dead, BAK::Direction::South).mImageIndices.size() - 1;
            it->mCombatantBAKLocation.mState = BAK::CombatantWorldState::Dead;
            it->mAnimationType = BAK::AnimationType::Dead;
            it->mFrame = deadFrameOffset;
            it->Update();
            mClickables.at(entityId).mEntityType = BAK::EntityType::DEAD_COMBATANT;
        }

        if (BAK::CombatIndex{combat.mCombatIndex} == BAK::MakalaCombat)
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
                    mGameState.SetDialogTextVariable(0, character.GetIndex().mValue);
                    deadCount++;
                }
                return BAK::Loop::Continue;
        });


        if (deadCount > 0)
        {
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
                if (BAK::IsSpecialBattle(BAK::CombatIndex{combat.mCombatIndex}))
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
    //return;
    mAccumulatedTime += timeDelta;
    if (mAccumulatedTime > .2)
    {
        mAccumulatedTime = 0;
        for (auto& combatant : mActiveCombatants)
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
            if (combat.mCombatIndex == combatIndex.mValue)
                return encounter;
        }
    }
    ASSERT(false);
}

void GameRunner::CleanCombatsOnNewZone()
{
    for (auto& combatant : mActiveCombatants)
    {
        mSystems->RemoveDynamicRenderable(combatant.mItemId);
        mClickables.erase(combatant.mItemId);
    }
    mActiveCombatants.clear();
    mCombatsToActiveCombatants.clear();

    for (auto& cwl : mGameState.GetCombatWorldLocations())
    {
        cwl.mPosition = BAK::GamePositionAndHeading{};
        cwl.mImageIndex = 0;
        cwl.mState = BAK::CombatantWorldState::Invisible1;
    }
}
}
