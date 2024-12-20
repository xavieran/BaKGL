#include "game/gameRunner.hpp"

#include "bak/combatModel.hpp"
#include "bak/entityType.hpp"
#include "game/interactable/factory.hpp"
#include "game/systems.hpp"

#include "bak/camera.hpp"
#include "bak/chapterTransitions.hpp"
#include "bak/coordinates.hpp"
#include "bak/dialog.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/encounter/teleport.hpp"
#include "bak/monster.hpp"
#include "bak/state/encounter.hpp"
#include "bak/state/event.hpp"
#include "bak/time.hpp"
#include "bak/types.hpp"
#include "bak/zone.hpp"

#include "com/logger.hpp"
#include "com/ostream.hpp"

#include "game/combatModelLoader.hpp"

#include "graphics/glm.hpp"
#include "gui/guiManager.hpp"
#include "gui/IDialogScene.hpp"

#include <utility>
#include <variant>

namespace Game {

GameRunner::GameRunner(
    Camera& camera,
    BAK::GameState& gameState,
    Gui::GuiManager& guiManager)
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
        std::nullopt,
        BAK::Inventory{0}},
    mSystems{nullptr},
    mSavedAngle{0},
    mTeleportFactory{},
    mClickablesEnabled{false},
    mLogger{Logging::LogState::GetLogger("Game::GameRunner")}
{
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
    mGameData = std::make_unique<BAK::GameData>(savePath);
    mGameState.LoadGameData(mGameData.get());
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
                && static_cast<BAK::CombatantWorldState>(cwl.mState) != BAK::CombatantWorldState::Dead)
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
            cwl.mState = combatant.mMovementType;
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

// Returns <combatActive, combatScouted>
std::pair<bool, bool> GameRunner::CheckCombatEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Combat& combat)
{
    mLogger.Debug() << __FUNCTION__ << " Checking combat active\n";
    if (!BAK::State::CheckCombatActive(mGameState, encounter, mGameState.GetZone()))
    {
        mLogger.Debug() << __FUNCTION__ << " Combat inactive\n";
        return std::make_pair(false, false);
    }

    if (!combat.mIsAmbush)
    {
        mLogger.Debug() << __FUNCTION__ << " Combat is not ambush\n";
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
            if (!BAK::State::CheckRecentlyEncountered(mGameState, encounter.GetIndex().mValue))
            {
                mGameState.Apply(BAK::State::SetRecentlyEncountered, encounter.GetIndex().mValue);
                auto chance = GetRandomNumber(0, 0xfff) % 100;
                const auto [character, scoutSkill] = mGameState.GetPartySkill(BAK::SkillType::Scouting, true);
                mLogger.Debug() << __FUNCTION__ << " Trying to scout combat: "
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
                mLogger.Debug() << __FUNCTION__ << " Combat was scouted already\n";
                return std::make_pair(true, false);
            }
        }
    }
}

void GameRunner::CheckAndDoCombatEncounter(
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
    auto timeDiff = (mGameState.GetWorldTime().GetTime()- timeOfScouting).mTime;
    if ((timeDiff / 0x1e) < 0x1e) // within scouting valid time
    {
        auto chance = GetRandomNumber(0, 0xfff) % 100;
        if (chance > lowestStealth)
        {
            // failed to sneak up
            mGameState.SetDialogContext_7530(1);
        }
        else
        {
            // Successfully snuck
            mGameState.GetParty().ImproveSkillForAll(
                BAK::SkillType::Stealth , BAK::SkillChange::ExercisedSkill, 1);
            mGameState.SetDialogContext_7530(0);
        }
    }
    else
    {
        mGameState.SetDialogContext_7530(2);
    }

    if (!combat.mCombatants.empty())
    {
        mGameState.SetMonster(BAK::MonsterIndex{combat.mCombatants.back().mMonster + 1u});
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

    BAK::State::SetPostCombatCombatSpecificFlags(mGameState, combat.mCombatIndex);
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
        const auto deadFrameOffset = monster.GetAnimation(BAK::AnimationType::Dead, BAK::Direction::South).mImageIndices.size() - 1;
        it->mCombatantBAKLocation.mState = std::to_underlying(BAK::CombatantWorldState::Dead);
        it->mAnimationType = BAK::AnimationType::Dead;
        it->mFrame = deadFrameOffset;
        it->Update();
        mClickables.at(entityId).mEntityType = BAK::EntityType::DEAD_COMBATANT;
    }
}

void GameRunner::DoBlockEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Block& block)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;

    mGuiManager.StartDialog(
            block.mDialog,
            false,
            false,
            &mDynamicDialogScene);

    mCamera.UndoPositionChange();
    mGameState.Apply(
        BAK::State::SetPostEnableOrDisableEventFlags,
        encounter,
        mGameState.GetZone());
}

void GameRunner::DoEventFlagEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::EventFlag& flag)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;

    if (flag.mEventPointer != 0)
        mGameState.SetEventValue(flag.mEventPointer, flag.mIsEnable ? 1 : 0);

    mGameState.Apply(
        BAK::State::SetPostEnableOrDisableEventFlags,
        encounter,
        mGameState.GetZone());
}

void GameRunner::DoZoneEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Zone& zone)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
        return;
    const auto& choices = BAK::DialogStore::Get().GetSnippet(zone.mDialog).mChoices;
    const bool isNoAffirmative = choices.size() == 2
        && std::holds_alternative<BAK::QueryChoice>(choices.begin()->mChoice)
        && std::get<BAK::QueryChoice>(choices.begin()->mChoice).mQueryIndex == BAK::Keywords::sNoIndex;
    mDynamicDialogScene.SetDialogFinished(
        [&, isNoAffirmative=isNoAffirmative, zone=zone](const auto& choice){
            // These dialogs should always result in a choice
            ASSERT(choice);
            Logging::LogDebug("Game::GameRunner") << "Switch to zone: " << zone << " got choice: " << choice << "\n";
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

void GameRunner::DoDialogEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::Dialog& dialog)
{
    if (!BAK::State::CheckEncounterActive(mGameState, encounter, mGameState.GetZone()))
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

    mGameState.Apply(
        BAK::State::SetPostDialogEventFlags,
        encounter,
        mGameState.GetZone());
}

void GameRunner::DoGDSEncounter(
    const BAK::Encounter::Encounter& encounter,
    const BAK::Encounter::GDSEntry& gds)
{
    // Pretty sure GDS encoutners will always happen...
    //if (!mGameState.Apply(BAK::State::CheckEncounterActive, encounter, mGameState.GetZone()))
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

            mGameState.Apply(BAK::State::SetPostGDSEventFlags, encounter);
            mDynamicDialogScene.ResetDialogFinished();
        });

    mGuiManager.StartDialog(
        gds.mEntryDialog,
        false,
        false,
        &mDynamicDialogScene);
}

void GameRunner::DoEncounter(const BAK::Encounter::Encounter& encounter)
{
    mLogger.Spam() << "Doing Encounter: " << encounter << "\n";
    std::visit(
        overloaded{
        [&](const BAK::Encounter::GDSEntry& gds){
            if (mGuiManager.InMainView())
                DoGDSEncounter(encounter, gds);
        },
        [&](const BAK::Encounter::Block& block){
            if (mGuiManager.InMainView())
                DoBlockEncounter(encounter, block);
        },
        [&](const BAK::Encounter::Combat& combat){
            if (mGuiManager.InMainView())
            {
                CheckAndDoCombatEncounter(encounter, combat);
            }
        },
        [&](const BAK::Encounter::Dialog& dialog){
            if (mGuiManager.InMainView())
                DoDialogEncounter(encounter, dialog);
        },
        [&](const BAK::Encounter::EventFlag& flag){
            if (mGuiManager.InMainView())
                DoEventFlagEncounter(encounter, flag);
        },
        [&](const BAK::Encounter::Zone& zone){
            if (mGuiManager.InMainView())
                DoZoneEncounter(encounter, zone);
        },
    },
    encounter.GetEncounter());
}

void GameRunner::CheckAndDoEncounter(glm::uvec2 position)
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
                DoEncounter(*mActiveEncounter);
        }
    }
}

void GameRunner::RunGameUpdate(bool advanceTime)
{
    if (mCamera.CheckAndResetDirty())
    {
        // This class var is only really necessary so that
        // we can us IMGUI to pull the latest triggered interactable.
        // Might want to clean this up.
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
            DoEncounter(*mActiveEncounter);
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
        cwl.mState = 1;
    }
}
}
