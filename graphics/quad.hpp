#pragma once

#include "graphics/meshObject.hpp"

#include <glm/glm.hpp>

#include <array>

namespace Graphics {

class Quad
{
public:
    Quad(
        std::array<glm::vec3, 4> corners,
        std::array<glm::vec3, 4> textureCoords);

    MeshObject ToMeshObject(float textureBlend) const;

private:
    std::array<glm::vec3, 4> mCorners;
    std::array<glm::vec3, 4> mTextureCoords;
    glm::vec3 mNormal;
};

}
