#pragma once

#include "com/logger.hpp"

#include "graphics/sphere.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/glm.hpp"

#include <algorithm>
#include <vector>
#include <unordered_map>


namespace Graphics {

class MeshObject
{
public:
    MeshObject(
        std::vector<glm::vec3> vertices,
        std::vector<glm::vec3> normals,
        std::vector<glm::vec4> colors,
        std::vector<glm::vec3> textureCoords,
        std::vector<float> textureBlends,
        std::vector<unsigned> indices)
    :
        mVertices{vertices},
        mNormals{normals},
        mColors{colors},
        mTextureCoords{textureCoords},
        mTextureBlends{textureBlends},
        mIndices{indices}
    {
    }

    unsigned long GetNumVertices() const
    {
        return mVertices.size();
    }

//private:
    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec4> mColors;
    std::vector<glm::vec3> mTextureCoords;
    // Ratio of texture to material color 
    // - solution to having textured and
    // non-textured objects (should I use diff shaders?)
    std::vector<float> mTextureBlends;
    std::vector<unsigned> mIndices;

};

MeshObject SphereToMeshObject(const Sphere& sphere, glm::vec4 color);

class MeshObjectStorage
{
public:
    using OffsetAndLength = std::pair<unsigned, unsigned>;

    MeshObjectStorage()
    :
        mOffset{0},
        mVertices{},
        mNormals{},
        mColors{},
        mTextureCoords{},
        mTextureBlends{},
        mIndices{}
    {
    }

    OffsetAndLength AddObject(
        const std::string& id,
        const MeshObject& obj)
    {
        if (mObjects.find(id) != mObjects.end())
        {
            mLog.Debug() << id << " already loaded" << std::endl;
            return GetObject(id);
        }

        auto length = obj.GetNumVertices();
        const auto offsetAndLength = std::make_pair(mOffset, length);

        mObjects.emplace(id, offsetAndLength);

        std::copy(obj.mVertices.begin(), obj.mVertices.end(), std::back_inserter(mVertices));
        std::copy(obj.mNormals.begin(), obj.mNormals.end(), std::back_inserter(mNormals));
        std::copy(obj.mColors.begin(), obj.mColors.end(), std::back_inserter(mColors));
        std::copy(obj.mTextureCoords.begin(), obj.mTextureCoords.end(), std::back_inserter(mTextureCoords));
        std::copy(obj.mTextureBlends.begin(), obj.mTextureBlends.end(), std::back_inserter(mTextureBlends));
        std::copy(obj.mIndices.begin(), obj.mIndices.end(), std::back_inserter(mIndices));

        mLog.Debug() << __FUNCTION__ << " " << id << " off: " << mOffset 
            << " len: " << length << std::endl;

        mOffset += obj.GetNumVertices();

        return offsetAndLength;
    }

    OffsetAndLength GetObject(const std::string& id)
    {   
        if (mObjects.find(id) == mObjects.end())
        {
            std::stringstream ss{};
            ss << "Couldn't find: " << id;
            throw std::runtime_error(ss.str());
        }
        return mObjects.find(id)->second;
    }

    std::size_t size() const
    {
        return mObjects.size();
    }

//private:
    unsigned long mOffset;
    std::unordered_map<std::string, OffsetAndLength> mObjects;

    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec4> mColors;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<float> mTextureBlends;
    std::vector<unsigned> mIndices;

    const Logging::Logger& mLog{
        Logging::LogState::GetLogger("MeshObjectStore")};
};

}
