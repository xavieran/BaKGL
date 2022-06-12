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
        unsigned textureIndex)
    :
        Quad{
            std::invoke([&](){
                const auto top = 0;
                const auto bottom = 1;
                const auto left = 0;
                const auto right = 1;

                return std::vector<glm::vec3>{
                    {left,  bottom, 0},
                    {left,  top,    0},
                    {right, top,    0},
                    {left,  bottom, 0},
                    {right, top,    0},
                    {right, bottom, 0}};
            }),
            std::invoke([&](){
                const auto maxU = width / maxDim;
                const auto maxV = height / maxDim;
                return std::vector<glm::vec3>{
                    {0,       0, textureIndex},
                    {0,    maxV, textureIndex},
                    {maxU, maxV, textureIndex},
                    {0,       0, textureIndex},
                    {maxU, maxV, textureIndex},
                    {maxU,    0, textureIndex}};

            }),
            {0, 1, 2, 3, 4, 5}}
    {}

    Quad(
        std::vector<glm::vec3> vertices,
        std::vector<glm::vec3> textureCoords,
        std::vector<unsigned> indices)
    :
        mVertices{vertices},
        mTextureCoords{textureCoords},
        mIndices{indices}
    {
    }

    Quad(
        glm::vec3 va,
        glm::vec3 vb,
        glm::vec3 vc,
        glm::vec3 vd)
    {
        const auto normal = glm::normalize(
            glm::cross(
                vd - va,
                vb - va));
        for (unsigned i = 0; i < 6; i++)
        {
            mNormals.emplace_back(normal);
            mIndices.emplace_back(i);
        }

        // Triangle one
        mVertices.emplace_back(vd);
        mVertices.emplace_back(va);
        mVertices.emplace_back(vb);

        mVertices.emplace_back(vd);
        mVertices.emplace_back(vb);
        mVertices.emplace_back(vc);
    }

    std::size_t GetNumVertices() const
    {
        return mVertices.size();
    }

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

    QuadStorage()
    :
        mOffset{0},
        mVertices{},
        mTextureCoords{},
        mIndices{}
    {
    }

    OffsetAndLength AddObject(
        const Quad& obj)
    {
        auto length = obj.GetNumVertices();

        auto& offsetAndLength = mObjects.emplace_back(mOffset, length);

        std::copy(obj.mVertices.begin(), obj.mVertices.end(), std::back_inserter(mVertices));
        std::copy(obj.mTextureCoords.begin(), obj.mTextureCoords.end(), std::back_inserter(mTextureCoords));
        std::copy(obj.mIndices.begin(), obj.mIndices.end(), std::back_inserter(mIndices));

        mOffset += obj.GetNumVertices();

        return offsetAndLength;
    }

    OffsetAndLength GetObject(std::size_t i) const
    {   
        if (i >= mObjects.size())
        {
            std::stringstream ss{};
            ss << "Couldn't find: " << i;
            throw std::runtime_error(ss.str());
        }
        return mObjects[i];
    }

    std::size_t size() const
    {
        return mObjects.size();
    }

//private:
    unsigned long mOffset;
    std::vector<OffsetAndLength> mObjects;

    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<unsigned> mIndices;
};

}
