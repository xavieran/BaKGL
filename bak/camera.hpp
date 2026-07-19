#pragma once

#include "bak/coordinates.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(
        unsigned width,
        unsigned height,
        float moveSpeed,
        float turnSpeed);
    
    glm::mat4 CalculateOrthoMatrix(unsigned width, unsigned height);
    glm::mat4 CalculatePerspectiveMatrix(unsigned width, unsigned height);
    void UseOrthoMatrix(unsigned width, unsigned height);
    void UsePerspectiveMatrix(unsigned width, unsigned height);

    void SetGameLocation(const BAK::GamePositionAndHeading& location);
    void SetSpeedScale(float scale);
    BAK::GamePositionAndHeading GetGameLocation() const;
    glm::uvec2 GetGameTile() const;
    void SetPosition(const glm::vec3& position);
    void SetHeight(float height);
    void SetAngle(glm::vec2 angle);
    BAK::GameHeading GetHeading();
    void SetDeltaTime(double dt);

    void MoveForward();
    void MoveBackward();
    void StrafeForward();
    void StrafeBackward();
    void StrafeRight();
    void StrafeLeft();
    void StrafeUp();
    void StrafeDown();

    bool HasPendingMove() const;
    glm::vec3 GetPendingPosition() const;
    void SetPendingDelta(const glm::vec3& delta);
    void AcceptPendingMove();
    void RejectPendingMove();

    void PositionChanged();
    void UndoPositionChange();

    void RotateLeft();
    void RotateRight();
    void RotateVerticalUp();
    void RotateVerticalDown();

    glm::vec3 GetDirection() const;
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;
    glm::vec2 GetAngle() const;
    BAK::GameHeading GetGameAngle() const;
    glm::mat4 GetViewMatrix() const;
    glm::vec3 GetPosition() const;
    glm::vec3 GetNormalisedPosition() const;
    const glm::mat4& GetProjectionMatrix() const;

    bool CheckAndResetDirty();
    unsigned GetAndClearUnitsTravelled();
    
private:
    float mMoveSpeed;
    float mTurnSpeed;
    float mSpeedScale{1.0f};
    float mDeltaTime;

    glm::vec3 mPosition;
    glm::vec3 mLastPosition;
    glm::vec3 mPendingDelta{};
    double mDistanceTravelled;
    glm::mat4 mProjectionMatrix;
    glm::vec2 mAngle;

    bool mDirty{};
};

