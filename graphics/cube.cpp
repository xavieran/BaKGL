#include "graphics/cube.hpp"

#include "graphics/meshObject.hpp"
#include "graphics/quad.hpp"

#include "com/assert.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace Graphics {

Cuboid::Cuboid(
    unsigned x_dim,
    unsigned y_dim,
    unsigned z_dim)
:
    mQuads{}
{
    const auto xd = x_dim / 2.;
    const auto yd = y_dim / 2.;
    const auto a = glm::vec3{-xd, 0, yd};
    const auto b = glm::vec3{xd, 0, yd};
    const auto c = glm::vec3{xd, 0, -yd};
    const auto d = glm::vec3{-xd, 0, -yd};
    const auto zd = glm::vec3{0, z_dim, 0};
    const auto e = a + zd;
    const auto f = b + zd;
    const auto g = c + zd;
    const auto h = d + zd;

    // 4 Sides to the quad...
    mQuads.emplace_back(a, e, h, d);
    mQuads.emplace_back(a, e, f, b);
    mQuads.emplace_back(b, f, g, c);
    mQuads.emplace_back(d, h, g, c);
    mQuads.emplace_back(h, g, f, e);
}

MeshObject Cuboid::ToMeshObject(glm::vec4 color) const
{
    auto vertices = std::vector<glm::vec3>{};
    auto normals  = std::vector<glm::vec3>{};
    auto indices  = std::vector<unsigned>{};

    unsigned off = 0;
    for (const auto& q : mQuads)
    {
        ASSERT(q.mVertices.size() == q.mNormals.size());
        ASSERT(q.mVertices.size() == q.mIndices.size());
        for (unsigned i = 0; i < q.GetNumVertices(); i++)
        {
            vertices.emplace_back(q.mVertices[i]);
            normals.emplace_back(q.mNormals[i]);
            indices.emplace_back(q.mIndices[i] + off);
        }
        off += q.GetNumVertices();
    }
    const auto textureCoords = std::vector<glm::vec3>(
        vertices.size(), glm::vec3{0});
    const auto colors = std::vector<glm::vec4>(
        vertices.size(), color);
    const auto textureBlends = std::vector<float>(
        vertices.size(), 0.0f);

    return MeshObject{
        vertices,
        normals,
        colors,
        textureCoords,
        textureBlends,
        indices
    };
}

}
