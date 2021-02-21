#pragma once

#include "constants.hpp"

#include "logger.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

class Renderable
{
    Renderable(
        glm::vec3 location,
        glm::vec3 rotation,
        glm::vec3 scale)
    :
        mLocation{location},
        mRotation{rotation},
        mScale{scale},
        mModelMatrix{CalculateModelMatrix()}
    {}

    const glm::mat4& GetModelMatrix()
    {
        return mModelMatrix;
    }

private:

    glm::mat4 CalculateModelMatrix()
    {
        auto modelMatrix = glm::mat4{1.0};
        modelMatrix = glm::translate(modelMatrix, mLocation / BAK::gWorldScale);
        modelMatrix = glm::scale(modelMatrix, mScale);
        // Dodgy... only works for rotation about y
        modelMatrix = glm::rotate(
            modelMatrix,
            inst.GetRotation().y,
            glm::vec3(0,1,0));

        return modelMatrix;
    }

    glm::vec3 mLocation;
    glm::vec3 mRotation;
    glm::vec3 mScale;

    glm::mat4 mModelMatrix;
}
