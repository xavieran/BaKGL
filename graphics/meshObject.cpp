#include "graphics/meshObject.hpp"

#include "graphics/sphere.hpp"

namespace Graphics {

MeshObject SphereToMeshObject(const Sphere& sphere, glm::vec4 color)
{
    std::vector<glm::vec3> vertices{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec4> colors{};
    std::vector<glm::vec3> textureCoords{};
    std::vector<float> textureBlends{};
    std::vector<unsigned> indices{};

    auto sVertices = sphere.getVertices();
    auto sNormals  = sphere.getNormals();
    auto sIndices = sphere.getIndices();

    for (unsigned i = 0; i < sphere.getIndexCount(); i++)
    {
        indices.emplace_back(i);
        vertices.emplace_back(
            sVertices[sIndices[i] * 3], 
            sVertices[sIndices[i] * 3 + 1], 
            sVertices[sIndices[i] * 3 + 2]);
        normals.emplace_back(
            sNormals[sIndices[i] * 3], 
            sNormals[sIndices[i] * 3 + 1], 
            sNormals[sIndices[i] * 3 + 2]);
        colors.emplace_back(color);
        textureCoords.emplace_back(0,0,0);
        textureBlends.emplace_back(0);

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
