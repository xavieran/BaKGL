#include "bak/camera.hpp"

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utility>

Camera::Camera(
    unsigned width,
    unsigned height,
    float moveSpeed,
    float turnSpeed)
:
    mMoveSpeed{moveSpeed},
    mTurnSpeed{turnSpeed},
    mDeltaTime{0},
    mPosition{0,1.4,0},
    mLastPosition{mPosition},
    mDistanceTravelled{0.0},
    mProjectionMatrix{CalculatePerspectiveMatrix(width, height)},
    mAngle{3.14, 0}
{}

glm::mat4 Camera::CalculateOrthoMatrix(unsigned width, unsigned height)
{
    const auto w = static_cast<float>(width);
    const auto h = static_cast<float>(height);
    return glm::ortho(
        -w,
        h,
        -w,
        h,
        1.0f,
        1000.0f
        //-1000.0f,
        //1000.0f
    );
}

glm::mat4 Camera::CalculatePerspectiveMatrix(unsigned width, unsigned height)
{
    return glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        1.0f,
        4000.0f
    );
}

void Camera::UseOrthoMatrix(unsigned width, unsigned height)
{
    mProjectionMatrix = CalculateOrthoMatrix(width, height);
}

void Camera::UsePerspectiveMatrix(unsigned width, unsigned height)
{
    mProjectionMatrix = CalculatePerspectiveMatrix(width, height);
}

void Camera::SetGameLocation(const BAK::GamePositionAndHeading& location)
{
    auto pos = BAK::ToGlCoord<float>(location.mPosition);
    //pos.y = mPosition.y;
    pos.y = 100;
    SetPosition(pos);
    SetAngle(BAK::ToGlAngle(location.mHeading));
}

BAK::GamePositionAndHeading Camera::GetGameLocation() const
{
    auto pos = glm::uvec2{mPosition.x, -mPosition.z};
    return BAK::GamePositionAndHeading{pos, GetGameAngle()};
}

glm::uvec2 Camera::GetGameTile() const
{
    const auto gamePosition = GetGameLocation();
    return BAK::GetTile(gamePosition.mPosition);
}

void Camera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
    PositionChanged();
    mDistanceTravelled = 0;
}

void Camera::SetAngle(glm::vec2 angle)
{
    mAngle = angle;
    mAngle.x = BAK::NormaliseRadians(mAngle.x);
    mAngle.y = BAK::NormaliseRadians(mAngle.y);
}

BAK::GameHeading Camera::GetHeading()
{
    // Make sure to normalise this between 0 and 1
    const auto bakAngle = BAK::ToBakAngle(mAngle.x);
    return static_cast<std::uint16_t>(bakAngle);
}

void Camera::SetDeltaTime(double dt)
{
    mDeltaTime = dt;
}

void Camera::MoveForward()
{
    PositionChanged();
    mPosition += GetDirection() * (mMoveSpeed * mDeltaTime);
}

void Camera::MoveBackward()
{
    PositionChanged();
    mPosition -= GetDirection() * (mMoveSpeed * mDeltaTime);
}

void Camera::StrafeForward()
{
    PositionChanged();
    mPosition += GetForward() * (mMoveSpeed * mDeltaTime);
}

void Camera::StrafeBackward()
{
    PositionChanged();
    mPosition -= GetForward() * (mMoveSpeed * mDeltaTime);
}

void Camera::StrafeRight()
{
    PositionChanged();
    mPosition += GetRight() * (mMoveSpeed * mDeltaTime);
}

void Camera::StrafeLeft()
{
    PositionChanged();
    mPosition -= GetRight() * (mMoveSpeed * mDeltaTime);
}

void Camera::StrafeUp()
{
    PositionChanged();
    mPosition += GetUp() * (mMoveSpeed * mDeltaTime);
}

void Camera::StrafeDown()
{
    PositionChanged();
    mPosition -= GetUp() * (mMoveSpeed * mDeltaTime);
}

void Camera::PositionChanged()
{
    mDistanceTravelled += std::abs(glm::distance(mPosition, mLastPosition));
    mLastPosition = mPosition;
    mDirty = true;
}

void Camera::UndoPositionChange()
{
    mPosition = mLastPosition;
}

void Camera::RotateLeft()
{
    SetAngle(
        glm::vec2{
            mAngle.x + mTurnSpeed * mDeltaTime,
            mAngle.y});
}

void Camera::RotateRight()
{
    SetAngle(
        glm::vec2{
            mAngle.x - mTurnSpeed * mDeltaTime,
            mAngle.y});
}

void Camera::RotateVerticalUp()
{
    SetAngle(
        glm::vec2{
            mAngle.x,
            mAngle.y + mTurnSpeed * mDeltaTime});
}

void Camera::RotateVerticalDown()
{
    SetAngle(
        glm::vec2{
            mAngle.x,
            mAngle.y - mTurnSpeed * mDeltaTime});
}

glm::vec3 Camera::GetDirection() const
{
    return {
        cos(mAngle.y) * sin(mAngle.x),
        sin(mAngle.y),
        cos(mAngle.y) * cos(mAngle.x)
    };
}

glm::vec3 Camera::GetForward() const
{
    return {
        cos(mAngle.x - 3.14f/2.0f),
        0,
        -sin(mAngle.x - 3.14f/2.0f)
    };
}

glm::vec3 Camera::GetRight() const
{
    return {
        sin(mAngle.x - 3.14f/2.0f),
        0,
        cos(mAngle.x - 3.14f/2.0f)
    };
}

glm::vec3 Camera::GetUp() const
{
    return glm::cross(GetRight(), GetDirection());
}

glm::vec2 Camera::GetAngle() const
{
    return mAngle;
}

BAK::GameHeading Camera::GetGameAngle() const
{
    return BAK::ToBakAngle(mAngle.x);
}

glm::mat4 Camera::GetViewMatrix() const
{ 
    return glm::lookAt(
        GetNormalisedPosition(),
        GetNormalisedPosition() + GetDirection(),
        GetUp()
    );
}

glm::vec3 Camera::GetPosition() const { return mPosition; }
glm::vec3 Camera::GetNormalisedPosition() const { return mPosition / BAK::gWorldScale; }
const glm::mat4& Camera::GetProjectionMatrix() const { return mProjectionMatrix; }

bool Camera::CheckAndResetDirty()
{
    return std::exchange(mDirty, false);
}

unsigned Camera::GetAndClearUnitsTravelled()
{
    auto unitsTravelled = static_cast<unsigned>(std::round(std::floor(mDistanceTravelled / 400.0)));
    if (unitsTravelled > 0)
    {
        mDistanceTravelled -= (unitsTravelled * 400);
        return unitsTravelled;
    }
    return 0;
}

