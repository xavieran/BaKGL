#pragma once

#include "constants.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(unsigned width, unsigned height, glm::vec3 worldCenter)
    :
        mPosition{0,1.4,0},
        mProjectionMatrix{
            glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(width) / static_cast<float>(height),
                1.0f,
                4000.0f
        )},
        mAngle{3.14, 0},
        mWorldCenter{worldCenter}
    {}

    void SetPosition(const glm::vec3& position)
    {
        mPosition = position;
    }
    
    void SetAngle(glm::vec2 angle)
    {
        mAngle = angle;
    }

    void MoveForward(float amount)
    {
        mPosition += GetDirection() * amount;
    }

    void StrafeRight(float amount)
    {
        mPosition += GetRight() * amount;
    }

    void StrafeUp(float amount)
    {
        mPosition += GetUp() * amount;
    }

    void RotateLeft(float amount)
    {
        mAngle.x += amount;
    }

    void RotateVertical(float amount)
    {
        mAngle.y += amount;
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
    glm::vec3 mPosition;
    glm::mat4 mProjectionMatrix;
    glm::vec2 mAngle;
    glm::vec3 mWorldCenter;
};
