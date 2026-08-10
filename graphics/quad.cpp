#include "graphics/quad.hpp"

#include <glm/glm.hpp>

#include <array>
#include <vector>

namespace Graphics {

Quad::Quad(
    std::array<glm::vec3, 4> corners,
    std::array<glm::vec3, 4> textureCoords)
:
    mCorners{corners},
    mTextureCoords{textureCoords},
    mNormal{
        glm::normalize(
            glm::cross(
                corners[0] - corners[1],
                corners[2] - corners[1]))}
{
}

MeshObject Quad::ToMeshObject(float textureBlend) const
{
    const auto order = std::array<unsigned, 6>{0, 1, 2, 0, 2, 3};

    auto vertices = std::vector<glm::vec3>{};
    auto normals = std::vector<glm::vec3>{};
    auto colors = std::vector<glm::vec4>{};
    auto textureCoords = std::vector<glm::vec3>{};
    auto textureBlends = std::vector<float>{};
    auto indices = std::vector<unsigned>{};

    for (unsigned i = 0; i < 6; i++)
    {
        vertices.emplace_back(mCorners[order[i]]);
        normals.emplace_back(mNormal);
        colors.emplace_back(glm::vec4{1.0f});
        textureCoords.emplace_back(mTextureCoords[order[i]]);
        textureBlends.emplace_back(textureBlend);
        indices.emplace_back(i);
    }

    return MeshObject{
        vertices,
        normals,
        colors,
        textureCoords,
        textureBlends,
        indices};
}

}
