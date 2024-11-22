#pragma once

#include "graphics/meshObject.hpp"
#include "graphics/quad.hpp"

#include "com/assert.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace Graphics {

//     ______
//   /|     /|
//  / |    / |
// /______/  /
// |      | /
// |______|/
// 
// This will be centered around it's x and y dimensions
// with its bottom face on the ground plane (0)
//
class Cuboid
{
public:
    Cuboid(
        unsigned x_dim,
        unsigned y_dim,
        unsigned z_dim);

    MeshObject ToMeshObject(glm::vec4 color) const;

    std::vector<Quad> mQuads;
};

}
