#pragma once


#include "src/logger.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>

namespace Graphics {

class Quad
{
public:
    Quad(double width, double height, double maxDim, unsigned textureIndex)
    :
        Quad{{
                {0,0,0},
                {height, 0, 0},
                {height, width, 0},
                {0, width, 0}
            },{
                {0,0,textureIndex},
                {height / maxDim, 0, textureIndex},
                {height / maxDim, width / maxDim, textureIndex},
                {0, width / maxDim, textureIndex}
            },{
                0, 1, 2, 0, 2, 3}}
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

    unsigned long GetNumVertices() const
    {
        return mVertices.size();
    }

//private:
    std::vector<glm::vec3> mVertices;
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

    OffsetAndLength GetObject(std::size_t i)
    {   
        if (i >= mObjects.size())
        {
            std::stringstream ss{};
            ss << "Couldn't find: " << i;
            throw std::runtime_error(ss.str());
        }
        return mObjects[i];
    }

//private:
    unsigned long mOffset;
    std::vector<OffsetAndLength> mObjects;

    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<unsigned> mIndices;

    const Logging::Logger& mLog{
        Logging::LogState::GetLogger("QuadStorage")};
};

}
