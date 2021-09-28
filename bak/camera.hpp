#pragma once

#include "constants.hpp"
#include "coordinates.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(
        unsigned width,
        unsigned height,
        float moveSpeed,
        float turnSpeed)
    :
        mMoveSpeed{moveSpeed},
        mTurnSpeed{turnSpeed},
        mDeltaTime{0},
        mPosition{0,1.4,0},
        mProjectionMatrix{
            glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(width) / static_cast<float>(height),
                1.0f,
                4000.0f
        )},
        mAngle{3.14, 0}
    {}

    void SetPosition(const glm::vec3& position)
    {
        mPosition = position;
    }
    
    void SetAngle(glm::vec2 angle)
    {
        mAngle = angle;
    }

    void SetDeltaTime(double dt)
    {
        mDeltaTime = dt;
    }

    void MoveForward()
    {
        mPosition += GetDirection() * (mMoveSpeed * mDeltaTime);
    }

    void MoveBackward()
    {
        mPosition -= GetDirection() * (mMoveSpeed * mDeltaTime);
    }

    void StrafeRight()
    {
        mPosition += GetRight() * (mMoveSpeed * mDeltaTime);
    }

    void StrafeLeft()
    {
        mPosition -= GetRight() * (mMoveSpeed * mDeltaTime);
    }

    void StrafeUp()
    {
        mPosition += GetUp() * (mMoveSpeed * mDeltaTime);
    }

    void StrafeDown()
    {
        mPosition -= GetUp() * (mMoveSpeed * mDeltaTime);
    }

    void RotateLeft()
    {
        mAngle.x += mTurnSpeed * mDeltaTime;
    }

    void RotateRight()
    {
        mAngle.x -= mTurnSpeed * mDeltaTime;
    }

    void RotateVerticalUp()
    {
        mAngle.y += mTurnSpeed * mDeltaTime;
    }

    void RotateVerticalDown()
    {
        mAngle.y -= mTurnSpeed * mDeltaTime;
    }

    glm::vec3 GetDirection() const
    {
        return {
            cos(mAngle.y) * sin(mAngle.x),
            sin(mAngle.y),
            cos(mAngle.y) * cos(mAngle.x)
        };
    }

    glm::vec3 GetRight() const
    {
        return {
            sin(mAngle.x - 3.14f/2.0f),
            0,
            cos(mAngle.x - 3.14f/2.0f)
        };
    }

    glm::vec3 GetUp() const
    {
        return glm::cross(GetRight(), GetDirection());
    }

    glm::vec2 GetAngle() const
    {
        return mAngle;
    }

    BAK::GameHeading GetGameAngle() const
    {
        const auto multiplier = static_cast<float>(0xff);
        return static_cast<BAK::GameHeading>(
            multiplier * (mAngle.x / (2.0f * glm::pi<float>())));
    }

    glm::mat4 GetViewMatrix() const
    { 
        return glm::lookAt(
            GetNormalisedPosition(),
            GetNormalisedPosition() + GetDirection(),
            GetUp()
        );
    }

    glm::vec3 GetPosition() const { return mPosition; }
    glm::vec3 GetNormalisedPosition() const { return mPosition / BAK::gWorldScale; }
    const glm::mat4& GetProjectionMatrix() const { return mProjectionMatrix; }

private:
    float mMoveSpeed;
    float mTurnSpeed;
    float mDeltaTime;

    glm::vec3 mPosition;
    glm::mat4 mProjectionMatrix;
    glm::vec2 mAngle;
};

