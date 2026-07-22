#pragma once

#include "bak/coordinates.hpp"
#include "bak/types.hpp"

#include "graphics/glm.hpp"

#include "com/logger.hpp"

#include <glm/glm.hpp>

#include <optional>
#include <unordered_map>

class Camera;

class Systems;

namespace BAK {
class GameState;
}

namespace Gui {
class GuiManager;
}

namespace Game {

using DoorLocationMap = std::unordered_map<glm::uvec2, BAK::DoorIndex, UVec2Hash>;

struct PitCross
{
    glm::uvec2 mLandingCell;
    BAK::GameHeading mHeading;
    unsigned mCellDistance;
};

class MovementManager
{
public:
    MovementManager(
        Camera& camera,
        BAK::GameState& gameState,
        Gui::GuiManager& guiManager);

    void SetSystems(Systems* systems);
    void SetDoorLocations(const DoorLocationMap* doorLocations);

    void MoveForward();
    void MoveBackward();
    void MoveLeft();
    void MoveRight();
    void SetSpeedScale(float scale);

    bool Update();

    bool CannotMoveHere(BAK::GamePosition playerPos) const;
    bool IsOnRoad(BAK::GamePosition playerPos) const;
    bool IsOnPit(glm::uvec2 pos) const;
    std::optional<PitCross> GetPitCross(
        BAK::GamePosition playerPos, glm::uvec2 pitLocation) const;
    std::optional<float> ComputeTerrainHeight(BAK::GamePosition playerPos) const;
    std::optional<BAK::GameHeading> GetOpenDirection(
        BAK::GamePositionAndHeading playerLocation, float distance, bool followRoad) const;

    void ToggleFollowRoad();
    void SetFollowRoadButtonVisible(bool visible);

    void RefreshAfterZoneLoad();
    void UpdateTerrainHeight();

    void SetClipEnabled(bool clip) { mClipEnabled = clip; }
    bool GetClipEnabled() const { return mClipEnabled; }
    void SetWallSlide(bool slide) { mWallSlide = slide; }
    bool GetWallSlide() const { return mWallSlide; }

private:
    std::optional<BAK::DoorIndex> GetDoorIndex(glm::uvec2 bakLocation) const;
    BAK::GamePosition GetFollowRoadCellCenter(BAK::GamePosition playerPos) const;
    std::optional<BAK::GamePosition> CanMoveFollowRoad(
        BAK::GamePositionAndHeading currentPos, unsigned distance) const;
    std::optional<BAK::GameHeading> GetOpenDirectionFollowRoad(
        BAK::GamePositionAndHeading playerLocation, float distance) const;
    void RotateTowardOpenHeading(
        BAK::GameHeading openHeading, BAK::GameHeading currentHeading);
    void CommitPendingMove();

    Camera& mCamera;
    BAK::GameState& mGameState;
    Gui::GuiManager& mGuiManager;
    Systems* mSystems{nullptr};
    const DoorLocationMap* mDoorLocations{nullptr};
    bool mClipEnabled{false};
    bool mWallSlide{false};

    const Logging::Logger& mLogger;
};

}
