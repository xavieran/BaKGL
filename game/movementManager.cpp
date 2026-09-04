#include "game/movementManager.hpp"

#include "bak/camera.hpp"
#include "bak/collision.hpp"
#include "bak/combat/combatModel.hpp"
#include "bak/constants.hpp"
#include "bak/coordinates.hpp"
#include "bak/gameState.hpp"
#include "bak/movement.hpp"
#include "bak/state/door.hpp"

#include "game/systems.hpp"

#include "gui/guiManager.hpp"
#include "gui/IMainView.hpp"

#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <algorithm>

namespace Game {

MovementManager::MovementManager(
    Camera& camera,
    BAK::GameState& gameState,
    Gui::GuiManager& guiManager)
:
    mCamera{camera},
    mGameState{gameState},
    mGuiManager{guiManager},
    mLogger{Logging::LogState::GetLogger("Game::MovementManager")}
{}

void MovementManager::SetSystems(Systems* systems)
{
    mSystems = systems;
}

void MovementManager::SetDoorLocations(const DoorLocationMap* doorLocations)
{
    mDoorLocations = doorLocations;
}

void MovementManager::SetSpeedScale(float scale)
{
    if (mGameState.IsUnderground())
    {
        scale /= 2;
    }
    mCamera.SetSpeedScale(scale);
}

std::optional<BAK::DoorIndex> MovementManager::GetDoorIndex(glm::uvec2 bakLocation) const
{
    if (!mDoorLocations)
    {
        return std::nullopt;
    }
    auto it = mDoorLocations->find(bakLocation);
    if (it != mDoorLocations->end())
    {
        return it->second;
    }
    return std::nullopt;
}

bool MovementManager::CannotMoveHere(BAK::GamePosition playerPos) const
{
    if (!mSystems)
    {
        return false;
    }

    const auto playerBakPos = glm::ivec2{playerPos};

    for (const auto& item : mSystems->GetNearbyCollisions(
            mSystems->GetAllowables(), playerBakPos, sMaxCollisionDistSq))
    {
        auto doorIndex = GetDoorIndex(item.GetBakLocation());
        if (doorIndex && !BAK::State::GetDoorState(mGameState, *doorIndex))
        {
            continue;
        }

        auto modelSpace = BAK::WorldToModelClipSpace(
            glm::vec2{playerBakPos},
            glm::vec2{item.GetBakLocation()},
            item.GetRotationY(),
            item.GetScale());

        if (BAK::PointInModelClip(modelSpace, item.GetModelClip()))
        {
            return false;
        }
    }

    for (const auto& item : mSystems->GetNearbyCollisions(
            mSystems->GetBlockables(), playerBakPos, sMaxCollisionDistSq))
    {
        auto doorIndex = GetDoorIndex(item.GetBakLocation());
        if (doorIndex && BAK::State::GetDoorState(mGameState, *doorIndex))
        {
            continue;
        }

        auto modelSpace = BAK::WorldToModelClipSpace(
            glm::vec2{playerBakPos},
            glm::vec2{item.GetBakLocation()},
            item.GetRotationY(),
            item.GetScale());

        if (BAK::PointInModelClip(modelSpace, item.GetModelClip()))
        {
            return true;
        }
    }

    return mGameState.GetFollowRoad() || mGameState.IsUnderground();
}

BAK::GamePosition MovementManager::GetFollowRoadCellCenter(BAK::GamePosition playerPos) const
{
    auto center = BAK::SnapPositionToCellCenter(playerPos);
    if (!IsOnRoad(center))
    {
        center = BAK::FindNearestRoadCell(
            playerPos,
            [this](BAK::GamePosition pos) { return IsOnRoad(pos); });
    }
    return center;
}

std::optional<BAK::GamePosition> MovementManager::CanMoveFollowRoad(
    BAK::GamePositionAndHeading currentPos, unsigned distance) const
{
    if (!mSystems)
    {
        return std::nullopt;
    }

    auto cellPos = BAK::GetCellPosition(currentPos.mPosition);

    // Can only move on a compass heading
    auto direction = BAK::HeadingToDirection(currentPos.mHeading);
    if (BAK::DirectionToHeading(direction) != currentPos.mHeading)
    {
        return std::nullopt;
    }

    auto thisCellPos = currentPos;
    thisCellPos.mPosition = BAK::SnapPositionToCellCenter(currentPos.mPosition);

    auto movedPosition = BAK::MoveForward(currentPos, distance);
    movedPosition.mPosition = BAK::SnapToCellVector(movedPosition.mPosition, direction);
    if (!IsOnRoad(movedPosition.mPosition))
    {
        return std::nullopt;
    }

    auto nextCellPos = BAK::MoveForward(thisCellPos, BAK::gCellSize);
    nextCellPos.mPosition = BAK::SnapPositionToCellCenter(nextCellPos.mPosition);

    if (IsOnRoad(nextCellPos.mPosition))
    {
        return movedPosition.mPosition;
    }

    const auto centerPos = thisCellPos.mPosition;

    const auto distanceToCenter = glm::dot(
        glm::vec2{centerPos} - glm::vec2{currentPos.mPosition},
        BAK::GetForwardVector(currentPos.mHeading));
    if (distanceToCenter <= 0)
    {
        return std::nullopt;
    }

    auto target = BAK::MoveForward(
        currentPos,
        std::min(static_cast<float>(distance), distanceToCenter));
    return BAK::SnapToCellVector(target.mPosition, direction);
}

bool MovementManager::IsOnRoad(BAK::GamePosition playerPos) const
{
    if (!mSystems)
    {
        return false;
    }

    const auto playerBakPos = glm::ivec2{playerPos};

    for (const auto& item : mSystems->GetNearbyCollisions(
            mSystems->GetAllowables(), playerBakPos, sMaxCollisionDistSq))
    {
        auto modelSpace = BAK::WorldToModelClipSpace(
            glm::vec2{playerBakPos},
            glm::vec2{item.GetBakLocation()},
            item.GetRotationY(),
            item.GetScale());

        if (BAK::PointInModelClip(modelSpace, item.GetModelClip()))
        {
            auto type = item.GetEntityType();
            return type == BAK::EntityType::EXTERIOR
                || type == BAK::EntityType::BRIDGE;
        }
    }

    return false;
}

bool MovementManager::IsOnPit(glm::uvec2 pos) const
{
    const auto playerPos = glm::ivec2{pos};

    for (const auto& item : mSystems->GetNearbyCollisions(
            mSystems->GetAllowables(), playerPos, sMaxCollisionDistSq))
    {
        if (item.GetEntityType() != BAK::EntityType::PIT)
        {
            continue;
        }

        auto modelSpace = BAK::WorldToModelClipSpace(
            glm::vec2{playerPos},
            glm::vec2{item.GetBakLocation()},
            item.GetRotationY(),
            item.GetScale());

        if (BAK::PointInModelClip(modelSpace, item.GetModelClip()))
        {
            return true;
        }
    }
    return false;
}

std::optional<PitCross> MovementManager::GetPitCross(
    BAK::GamePosition playerPos, glm::uvec2 pitLocation) const
{
    const auto pitCellPos = BAK::SnapPositionToCellCenter(pitLocation);
    const auto partyCellPos = BAK::SnapPositionToCellCenter(playerPos);

    const auto heading = BAK::GetHeadingBetween(partyCellPos, pitCellPos);
    const auto direction = BAK::HeadingToDirection(heading);
    if (!BAK::IsCardinal(direction))
    {
        mLogger.Debug() << __FUNCTION__ << " Not adjacent to pit direction is: "
            << BAK::ToString(direction) << "\n";
        return std::nullopt;
    }

    const auto distance = glm::distance(glm::vec2{partyCellPos}, glm::vec2{pitCellPos});
    const auto cellDistance = static_cast<unsigned>(distance / BAK::gCellSize);

    if (distance > BAK::gCellSize * 2)
    {
        mLogger.Debug() << __FUNCTION__ << " Too far from pit: " << distance
            << " " << cellDistance << "\n";
        return std::nullopt;
    }
    const auto step = BAK::ToDelta(direction) * static_cast<int>(BAK::gCellSize);

    auto landingCell = glm::uvec2{glm::ivec2{pitCellPos}};
    unsigned i = 0;
    while (IsOnPit(landingCell))
    {
        landingCell = glm::uvec2{glm::ivec2{landingCell} + step};
        i++;
        if (i > 4)
        {
            mLogger.Error() << __FUNCTION__ << " Too many iterations to traverse pit!\n";
            break;
        }
    }

    mLogger.Spam() << __FUNCTION__ << " Pit cell center: " << pitCellPos
        << " party cell center: " << partyCellPos
        << " heading: " << heading
        << " landing: " << landingCell
        << " cell distance: " << cellDistance << "\n";

    return PitCross{landingCell, heading, cellDistance};
}

void MovementManager::SetFollowRoadButtonVisible(bool visible)
{
    mGuiManager.GetMainView().SetFollowRoadVisible(visible);
    if (visible)
    {
        mGuiManager.GetMainView().SetFollowRoadActive(mGameState.GetFollowRoad());
    }
}

void MovementManager::UpdateTerrainHeight()
{
    mCamera.SetHeight(
        ComputeTerrainHeight(mCamera.GetGameLocation().mPosition));
}

void MovementManager::RefreshAfterZoneLoad()
{
    UpdateTerrainHeight();
    SetFollowRoadButtonVisible(IsOnRoad(mGameState.GetLocation().mPosition));
}

void MovementManager::ToggleFollowRoad()
{
    mGameState.SetFollowRoad(!mGameState.GetFollowRoad());
    mGuiManager.GetMainView().SetFollowRoadActive(mGameState.GetFollowRoad());

    if (mGameState.GetFollowRoad())
    {
        auto location = mCamera.GetGameLocation();
        location.mPosition = BAK::FindNearestRoadCell(
            location.mPosition,
            [this](BAK::GamePosition pos) { return IsOnRoad(pos); });

        mCamera.SetGameLocation(location);
        UpdateTerrainHeight();
        auto playerPos = mCamera.GetGameLocation();
        auto cellPos = playerPos.mPosition % static_cast<unsigned>(BAK::gCellSize);
    }
}

void MovementManager::ClearFollowRoad()
{
    mGameState.SetFollowRoad(false);
    mGuiManager.GetMainView().SetFollowRoadActive(false);
}

float MovementManager::ComputeTerrainHeight(BAK::GamePosition playerPos) const
{
    if (!mSystems)
    {
        return mDefaultHeight;
    }

    const auto playerBakPos = glm::ivec2{playerPos};

    for (const auto& item : mSystems->GetNearbyCollisions(
            mSystems->GetAllowables(), playerBakPos, sMaxCollisionDistSq))
    {
        auto modelSpace = BAK::WorldToModelClipSpace(
            glm::vec2{playerBakPos},
            glm::vec2{item.GetBakLocation()},
            item.GetRotationY(),
            item.GetScale());

        auto height = BAK::ComputeHeight(modelSpace, item.GetModelClip());
        if (height)
        {
            return BAK::ComputeWorldHeight(
                *height,
                item.GetScale(),
                mDefaultHeight);
        }
    }

    return mDefaultHeight;
}

std::optional<BAK::GameHeading> MovementManager::GetOpenDirection(
    BAK::GamePositionAndHeading playerLocation, float distance, bool followRoad) const
{
    mLogger.Debug() << __FUNCTION__ << " Input: " << playerLocation << " distance: " << distance << " followRoad: " << followRoad << "\n";

    if (!mSystems)
    {
        mLogger.Debug() << __FUNCTION__ << " No systems, returning nullopt\n";
        return std::nullopt;
    }

    if (followRoad)
    {
        return GetOpenDirectionFollowRoad(playerLocation, distance);
    }

    const auto currentHeading = BAK::SnapHeading(
        playerLocation.mHeading, BAK::gBakSmallRotationBakHeading);

    std::int16_t leftStep  = BAK::gBakSmallRotationBakHeading;
    leftStep = -leftStep;
    std::int16_t rightStep = BAK::gBakSmallRotationBakHeading;
    std::int16_t negativeNinetyDegrees = BAK::gBakNinetyDegrees;
    negativeNinetyDegrees = -negativeNinetyDegrees;
    auto maxSearchAngleLeft  = BAK::RotateHeading(currentHeading, negativeNinetyDegrees);
    auto maxSearchAngleRight = BAK::RotateHeading(currentHeading, BAK::gBakNinetyDegrees);

    mLogger.Debug() << __FUNCTION__ << " currentHeading: " << currentHeading
        << " (" << BAK::ToString(BAK::HeadingToDirection(currentHeading)) << ")"
        << " leftStep: " << leftStep
        << " rightStep: " << rightStep
        << " maxSearchAngleLeft: " << maxSearchAngleLeft
        << " (" << BAK::ToString(BAK::HeadingToDirection(maxSearchAngleLeft)) << ")"
        << " maxSearchAngleRight: " << maxSearchAngleRight
        << " (" << BAK::ToString(BAK::HeadingToDirection(maxSearchAngleRight)) << ")"
        << "\n";

    auto currentSearchLeft  = currentHeading;
    auto currentSearchRight = currentHeading;

    auto headingToCheck = playerLocation;
    std::optional<BAK::GameHeading> openLeft;
    std::optional<BAK::GameHeading> openRight;

    unsigned iteration = 0;
    while (currentSearchLeft != maxSearchAngleLeft)
    {
        mLogger.Debug() << __FUNCTION__ << " [" << iteration << "]"
            << " currentSearchLeft: " << currentSearchLeft
            << " (" << BAK::ToString(BAK::HeadingToDirection(currentSearchLeft)) << ")"
            << " currentSearchRight: " << currentSearchRight
            << " (" << BAK::ToString(BAK::HeadingToDirection(currentSearchRight)) << ")"
            << "\n";

        currentSearchLeft = BAK::RotateHeading(currentSearchLeft, leftStep);
        headingToCheck.mHeading = currentSearchLeft;
        auto positionToCheck = BAK::MoveForward(headingToCheck, distance);

        mLogger.Debug() << __FUNCTION__ << " [" << iteration << "]"
            << " Left check: heading=" << currentSearchLeft
            << " (" << BAK::ToString(BAK::HeadingToDirection(currentSearchLeft)) << ")"
            << " posToCheck=" << positionToCheck
            << " cannotMove=" << CannotMoveHere(positionToCheck.mPosition)
            << "\n";

        if (!CannotMoveHere(positionToCheck.mPosition))
        {
            openLeft = currentSearchLeft;
            mLogger.Debug() << __FUNCTION__ << " [" << iteration << "]"
                << " Left OPEN at heading " << *openLeft
                << " (" << BAK::ToString(BAK::HeadingToDirection(*openLeft)) << ")"
                << "\n";
        }

        currentSearchRight = BAK::RotateHeading(currentSearchRight, rightStep);
        headingToCheck.mHeading = currentSearchRight;
        positionToCheck = BAK::MoveForward(headingToCheck, distance);

        mLogger.Debug() << __FUNCTION__ << " [" << iteration << "]"
            << " Right check: heading=" << currentSearchRight
            << " (" << BAK::ToString(BAK::HeadingToDirection(currentSearchRight)) << ")"
            << " posToCheck=" << positionToCheck
            << " cannotMove=" << CannotMoveHere(positionToCheck.mPosition)
            << "\n";

        if (!CannotMoveHere(positionToCheck.mPosition))
        {
            openRight = currentSearchRight;
            mLogger.Debug() << __FUNCTION__ << " [" << iteration << "]"
                << " Right OPEN at heading " << *openRight
                << " (" << BAK::ToString(BAK::HeadingToDirection(*openRight)) << ")"
                << "\n";
        }

        if (openLeft || openRight)
        {
            mLogger.Debug() << __FUNCTION__ << " [" << iteration << "]"
                << " Found open direction, breaking"
                << " openLeft=" << (openLeft ? std::to_string(*openLeft) : "nullopt")
                << " openRight=" << (openRight ? std::to_string(*openRight) : "nullopt")
                << "\n";
            break;
        }

        iteration++;
    }

    if (openLeft && openRight)
    {
        mLogger.Debug() << __FUNCTION__ << " Both directions open (left=" << *openLeft
            << " right=" << *openRight << "), returning nullopt\n";
        return std::nullopt;
    }
    else if (openLeft)
    {
        mLogger.Debug() << __FUNCTION__ << " Returning left heading " << *openLeft
            << " (" << BAK::ToString(BAK::HeadingToDirection(*openLeft)) << ")\n";
        return *openLeft;
    }
    else if (openRight)
    {
        mLogger.Debug() << __FUNCTION__ << " Returning right heading " << *openRight
            << " (" << BAK::ToString(BAK::HeadingToDirection(*openRight)) << ")\n";
        return *openRight;
    }

    mLogger.Debug() << __FUNCTION__ << " No open direction found after " << iteration
        << " iterations, returning nullopt\n";
    return std::nullopt;
}

std::optional<BAK::GameHeading> MovementManager::GetOpenDirectionFollowRoad(
    BAK::GamePositionAndHeading playerLocation, float distance) const
{
    // 45 degree angle
    constexpr auto gDirectionStep = 32;

    const auto currentHeading = playerLocation.mHeading;
    const auto base = static_cast<BAK::GameHeading>((currentHeading / gDirectionStep) * gDirectionStep);
    const auto next = static_cast<BAK::GameHeading>((base + gDirectionStep) % BAK::gBakHeadingFullCircle);

    const auto distToBase = currentHeading - base;
    const auto distToNext = gDirectionStep - distToBase;
    const auto nearest = distToBase <= distToNext ? base : next;

    mLogger.Debug() << __FUNCTION__ << " current heading=" << currentHeading
        << " (" << BAK::ToString(BAK::HeadingToDirection(currentHeading)) << ")"
        << " nearest cardinal=" << nearest
        << " (" << BAK::ToString(BAK::HeadingToDirection(nearest)) << ")"
        << "\n";

    auto headingToCheck = playerLocation;

    const auto checkDirectionOpen = [&](BAK::GameHeading candidate)
    {
        headingToCheck.mHeading = candidate;
        auto pos = BAK::MoveForward(headingToCheck, distance);
        pos.mPosition = BAK::SnapPositionToCellCenter(pos.mPosition);
        const bool canMove = IsOnRoad(pos.mPosition);

        mLogger.Debug() << __FUNCTION__ << " checkDirectionOpen: heading=" << candidate
            << " (" << BAK::ToString(BAK::HeadingToDirection(candidate)) << ")"
            << " posToCheck=" << pos
            << " canMove=" << canMove
            << "\n";
        return canMove;
    };

    std::optional<BAK::GameHeading> openLeft;
    std::optional<BAK::GameHeading> openRight;

    for (auto step = 0; step <= 2; step++)
    {
        const auto right = static_cast<BAK::GameHeading>(
            (nearest + step * gDirectionStep) % BAK::gBakHeadingFullCircle);
        const auto left = static_cast<BAK::GameHeading>(
            (nearest + BAK::gBakHeadingFullCircle - step * gDirectionStep) % BAK::gBakHeadingFullCircle);

        if (step == 0)
        {
            // first step => pointing ahead, so return immediately
            if (checkDirectionOpen(right))
            {
                return right;
            }
            continue;
        }

        if (!openRight && checkDirectionOpen(right))
        {
            openRight = right;
        }

        if (!openLeft && checkDirectionOpen(left))
        {
            openLeft = left;
        }

        // If both directions are open the game stops and forces the user
        // to pick a direction, e.g. you are coming up a T intersection
        if (openLeft && openRight)
        {
            return std::nullopt;
        }
    }

    if (openLeft)
    {
        return *openLeft;
    }

    if (openRight)
    {
        return *openRight;
    }

    return std::nullopt;
}

void MovementManager::RotateTowardOpenHeading(
    BAK::GameHeading openHeading,
    BAK::GameHeading currentHeading)
{
    const auto oldHeading = mCamera.GetHeading();

    if (BAK::GetRotationDirection(currentHeading, openHeading) == BAK::CardinalDirection::West)
    {
        mCamera.RotateLeft();
    }
    else
    {
        mCamera.RotateRight();
    }

    const auto newHeading = mCamera.GetHeading();
    if (const auto snapped = BAK::SnapHeadingIfOvershot(oldHeading, newHeading, openHeading))
    {
        auto glAngle = BAK::ToGlAngle(*snapped);
        glAngle.y = mCamera.GetAngle().y;
        mCamera.SetAngle(glAngle);
    }
}

void MovementManager::CommitPendingMove()
{
    mCamera.AcceptPendingMove();
    UpdateTerrainHeight();
    SetFollowRoadButtonVisible(IsOnRoad(mCamera.GetGameLocation().mPosition));
}

void MovementManager::ApplyPendingTurn()
{
    const auto target = *mPendingTurnHeading;
    const auto oldHeading = mCamera.GetHeading();

    const auto delta = sAutoRotateSpeed * mCamera.GetDeltaTime();
    auto angle = mCamera.GetAngle();

    if (mPendingTurnDirection == BAK::CardinalDirection::West)
    {
        angle.x = BAK::NormaliseRadians(angle.x + delta);
    }
    else
    {
        angle.x = BAK::NormaliseRadians(angle.x - delta);
    }
    mCamera.SetAngle(angle);

    const auto newHeading = mCamera.GetHeading();
    if (const auto snapped = BAK::SnapHeadingIfOvershot(oldHeading, newHeading, target))
    {
        auto snappedAngle = BAK::ToGlAngle(*snapped);
        snappedAngle.y = mCamera.GetAngle().y;
        mCamera.SetAngle(snappedAngle);
    }

    if (mCamera.GetHeading() == target)
    {
        mPendingTurnHeading.reset();
    }
}

void MovementManager::MoveLeft()
{
    if (mGameState.GetFollowRoad())
    {
        return;
    }
    mCamera.StrafeLeft();
}

void MovementManager::MoveRight()
{
    if (mGameState.GetFollowRoad())
    {
        return;
    }
    mCamera.StrafeRight();
}

void MovementManager::SetDefaultHeight(float height)
{
    mDefaultHeight = height;
}

float MovementManager::GetDefaultHeight() const
{
    return mDefaultHeight;
}

void MovementManager::MoveForward(bool strafe)
{
    if (mPendingTurnHeading)
    {
        return;
    }

    if (!mGameState.GetFollowRoad())
    {
        if (strafe)
        {
            mCamera.StrafeForward();
        }
        else
        {
            mCamera.MoveForward();
        }
        return;
    }

    const auto distance = static_cast<unsigned>(
        mCamera.GetScaledMoveSpeed() * mCamera.GetDeltaTime());
    const auto location = mCamera.GetGameLocation();

    const auto movePos = CanMoveFollowRoad(location, distance);
    if (movePos && *movePos != location.mPosition)
    {
        auto delta = BAK::ToGlCoord<float>(*movePos) - mCamera.GetPosition();
        delta.y = 0.0f;
        mCamera.SetPendingDelta(delta);
        return;
    }

    auto thisCellPos = location;
    thisCellPos.mPosition = GetFollowRoadCellCenter(location.mPosition);
    const auto openHeading = GetOpenDirection(thisCellPos, BAK::gCellSize, true);
    if (!openHeading || location.mHeading == *openHeading)
    {
        return;
    }

    // If we can't move forward, but there's a valid open direction from the
    // cell center, recenter and turn towards it.
    auto glPos = BAK::ToGlCoord<float>(thisCellPos.mPosition);
    glPos.y = mCamera.GetPosition().y;
    mCamera.SetPosition(glPos);
    UpdateTerrainHeight();
    RotateTowardOpenHeading(*openHeading, mCamera.GetHeading());
}

void MovementManager::MoveBackward(bool strafe)
{
    if (!mGameState.GetFollowRoad())
    {
        if (strafe)
        {
            mCamera.StrafeBackward();
        }
        else
        {
            mCamera.MoveBackward();
        }
        return;
    }

    const auto distance = static_cast<unsigned>(
        mCamera.GetScaledMoveSpeed() * mCamera.GetDeltaTime());
    const auto location = mCamera.GetGameLocation();

    auto backPos = location;
    backPos.mHeading = static_cast<BAK::GameHeading>(
        (location.mHeading + BAK::gBakHeadingHalfCircle) % BAK::gBakHeadingFullCircle);

    const auto movePos = CanMoveFollowRoad(backPos, distance);
    if (movePos && *movePos != location.mPosition)
    {
        auto delta = BAK::ToGlCoord<float>(*movePos) - mCamera.GetPosition();
        delta.y = 0.0f;
        mCamera.SetPendingDelta(delta);
    }
    else
    {
        // BaK doesn't do auto-turn when moving backwards in follow road mode
    }
}

bool MovementManager::Update()
{
    if (mPendingTurnHeading)
    {
        mCamera.RejectPendingMove();
        ApplyPendingTurn();
        return true;
    }

    if (!mCamera.HasPendingMove())
    {
        return false;
    }

    const auto gameLocation = mCamera.GetGameLocation();
    const auto targetPos = mCamera.GetPendingPosition();
    const auto bakTargetPos = glm::uvec2{targetPos.x, -targetPos.z};

    if (!CannotMoveHere(bakTargetPos) || !GetClipEnabled())
    {
        CommitPendingMove();
        return true;
    }

    if (mGameState.GetFollowRoad())
    {
        mCamera.RejectPendingMove();
        return false;
    }

    const auto openHeading = GetOpenDirection(
        gameLocation, BAK::gRotationSearchDistance, false);

    if (openHeading && GetWallSlide())
    {
        const auto originalDelta = mCamera.GetPendingPosition() - mCamera.GetPosition();
        auto [projected, bakProjectedPos] = BAK::ProjectSlide(
            originalDelta, mCamera.GetPosition(), *openHeading,
            BAK::gRotationSearchDistance);

        mCamera.RejectPendingMove();
        RotateTowardOpenHeading(*openHeading, gameLocation.mHeading);

        if (!CannotMoveHere(bakProjectedPos))
        {
            mCamera.SetPendingDelta(projected);
            CommitPendingMove();
            return true;
        }
        return false;
    }

    if (openHeading)
    {
        if (mGameState.IsUnderground())
        {
            mPendingTurnHeading = *openHeading;
            mPendingTurnDirection = BAK::GetRotationDirection(
                gameLocation.mHeading, *openHeading);
        }
        else
        {
            RotateTowardOpenHeading(*openHeading, gameLocation.mHeading);
        }
    }
    else
    {
        mLogger.Debug() << "Could move in either direction or not at all\n";
    }

    mCamera.RejectPendingMove();
    return false;
}

}
