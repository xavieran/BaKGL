#pragma once

#include "com/logger.hpp"

#include "graphics/glm.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <functional>
#include <vector>

namespace Graphics {

class Quad
{
public:
    Quad(
        double width,
        double height,
        double maxDim,
        unsigned textureIndex);

    Quad(
        std::vector<glm::vec3> vertices,
        std::vector<glm::vec3> textureCoords,
        std::vector<unsigned> indices);

    Quad(
        glm::vec3 va,
        glm::vec3 vb,
        glm::vec3 vc,
        glm::vec3 vd);

    std::size_t GetNumVertices() const;

//private:
    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<unsigned> mIndices;
};

class QuadStorage
{
public:
    using OffsetAndLength = std::pair<unsigned, unsigned>;

    QuadStorage();

    OffsetAndLength AddObject(
        const Quad& obj);

    OffsetAndLength GetObject(std::size_t i) const;
    std::size_t size() const;

//private:
    unsigned long mOffset;
    std::vector<OffsetAndLength> mObjects;

    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<unsigned> mIndices;
};

}
