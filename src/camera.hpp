#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(unsigned width, unsigned height)
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
        mVerticalAngle{0}
    {}
    
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
            mPosition,
            mPosition + GetDirection(),
            GetUp()
        );
    }

    const glm::vec3 GetPosition() const { return mPosition; }
    const glm::mat4& GetProjectionMatrix() const { return mProjectionMatrix; }

private:
    glm::vec3 mPosition;
    glm::mat4 mProjectionMatrix;
    double mHorizontalAngle;
    double mVerticalAngle;
};
