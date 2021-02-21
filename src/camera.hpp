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
        mHorizontalAngle{3.14},
        mVerticalAngle{0},
        mWorldCenter{worldCenter}
    {}

    void SetPosition(const glm::vec3& position)
    {
        mPosition = position;
    }
    
    void MoveForward(float amount)
    {
        mPosition += GetDirection() * amount;
    }

    void StrafeRight(float amount)
    {
        mPosition += GetRight() * amount;
    }

    void RotateLeft(float amount)
    {
        mHorizontalAngle += amount;
    }

    void RotateVertical(float amount)
    {
        mVerticalAngle += amount;
    }

    glm::vec3 GetDirection() const
    {
        return {
            cos(mVerticalAngle) * sin(mHorizontalAngle),
            sin(mVerticalAngle),
            cos(mVerticalAngle) * cos(mHorizontalAngle)
        };
    }

    glm::vec3 GetRight() const
    {
        return {
            sin(mHorizontalAngle - 3.14f/2.0f),
            0,
            cos(mHorizontalAngle - 3.14f/2.0f)
        };
    }

    glm::vec3 GetUp() const
    {
        return glm::cross(GetRight(), GetDirection());
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
    double mHorizontalAngle;
    double mVerticalAngle;
    glm::vec3 mWorldCenter;
};
