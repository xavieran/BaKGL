#pragma once

#include "worldFactory.hpp"
#include "logger.hpp"

#include "Palette.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <algorithm>


namespace BAK {


class MeshObject
{
public:
    MeshObject()
    :
        mVertices{},
        mNormals{},
        mColors{},
        mTextureCoords{},
        mTextureBlends{},
        mIndices{}
    {
    }

    void LoadFromBaKItem(
        const ZoneItem& item,
        const TextureStore& store,
        const Palette& pal)
    {
        constexpr float scale = 100;
        auto glmVertices = std::vector<glm::vec3>{};

        for (const auto& vertex : item.GetDatItem().mVertices)
        {
            glmVertices.emplace_back(
                vertex.GetX() / scale,
                vertex.GetZ() / scale, 
                vertex.GetY() / scale);
        }

        for (const auto& faceV : item.GetDatItem().mFaces | boost::adaptors::indexed())
        {
            const auto& face = faceV.value();
            const auto i = faceV.index();
            unsigned triangles = face.size() - 2;
            
            // Tesselate the face
            // Generate normals and new indices for each face vertex
            for (unsigned triangle = 0; triangle < triangles; triangle++)
            {
                auto i_a = face[0];
                auto i_b = face[triangle + 1];
                auto i_c = face[triangle + 2];

                mVertices.emplace_back(glmVertices[i_a]);
                mIndices.emplace_back(mVertices.size() - 1);
                mVertices.emplace_back(glmVertices[i_b]);
                mIndices.emplace_back(mVertices.size() - 1);
                mVertices.emplace_back(glmVertices[i_c]);
                mIndices.emplace_back(mVertices.size() - 1);
                
                // Hacky - only works for quads - but the game only
                // textures quads anyway...
                auto colorIndex = item.GetDatItem().mColors[i];
                auto textureIndex = colorIndex;

                float u = 1.0;
                float v = 1.0;

                auto maxDim = store.GetMaxDim();

                static constexpr std::uint8_t texturePalette = 0x91;
                static constexpr std::uint8_t terrainPalette = 0xc1;

                if (item.GetDatItem().mPalettes[i] == texturePalette
                    || item.GetDatItem().mPalettes[i] == 0x90)
                {
                    mTextureBlends.emplace_back(1.0);
                    mTextureBlends.emplace_back(1.0);
                    mTextureBlends.emplace_back(1.0);
                }
                else if (item.GetDatItem().mPalettes[i] == terrainPalette)
                {
                    mTextureBlends.emplace_back(1.0);
                    mTextureBlends.emplace_back(1.0);
                    mTextureBlends.emplace_back(1.0);
                    if (item.GetName().substr(0, 1) == "r")
                    {
                        if (textureIndex == 3)
                            textureIndex = 5;
                        else if (textureIndex == 5)
                            textureIndex = 6;
                    }
                    if (item.GetName().substr(0, 1) == "t")
                    {
                        if (textureIndex == 2)
                            textureIndex = 3;
                        else if (textureIndex == 5)
                            textureIndex = 6;
                    }
                }
                else if (textureIndex < 7)
                {
                    mTextureBlends.emplace_back(1.0);
                    mTextureBlends.emplace_back(1.0);
                    mTextureBlends.emplace_back(1.0);
                    textureIndex += store.GetTerrainOffset();
                }
                else
                {
                    mTextureBlends.emplace_back(0.0);
                    mTextureBlends.emplace_back(0.0);
                    mTextureBlends.emplace_back(0.0);
                }

                if (colorIndex < store.GetTextures().size())
                {
                    u = static_cast<float>(store.GetTexture(textureIndex).mWidth) 
                        / static_cast<float>(maxDim);
                    v = static_cast<float>(store.GetTexture(textureIndex).mHeight) 
                        / static_cast<float>(maxDim);
                }

                if (triangle == 0)
                {
                    mTextureCoords.emplace_back(u  , v,   textureIndex);
                    mTextureCoords.emplace_back(0.0, v,   textureIndex);
                    mTextureCoords.emplace_back(0.0, 0.0, textureIndex);
                }
                else
                {
                    mTextureCoords.emplace_back(u,   v,   textureIndex);
                    mTextureCoords.emplace_back(0.0, 0.0, textureIndex);
                    mTextureCoords.emplace_back(u,   0.0, textureIndex);
                }

                auto normal = glm::normalize(
                    glm::cross(
                        glmVertices[i_a] - glmVertices[i_b],
                        glmVertices[i_a] - glmVertices[i_c]));

                mNormals.emplace_back(normal);
                mNormals.emplace_back(normal);
                mNormals.emplace_back(normal);
                
                auto color = pal.GetColor(colorIndex);
                auto glmCol = \
                    glm::vec3(
                        static_cast<float>(color.r) / 256,
                        static_cast<float>(color.g) / 256,
                        static_cast<float>(color.b) / 256);

                mColors.emplace_back(glmCol);
                mColors.emplace_back(glmCol);
                mColors.emplace_back(glmCol);
            }
        }
    }

    unsigned long GetNumVertices() const
    {
        return mVertices.size();
    }

//private:
    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec3> mColors;
    std::vector<glm::vec3> mTextureCoords;
    // Ratio of texture to material color 
    // - lame solution to having textured 
    // and non-textured objects (should I use diff shaders?)
    std::vector<glm::vec3> mTextureBlends;
	std::vector<unsigned> mIndices;

    const Logging::Logger& mLog{
        Logging::LogState::GetLogger("MeshObject")};
};

class MeshObjectStorage
{
public:
    using OffsetAndLength = std::pair<unsigned long, unsigned long>;

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

    OffsetAndLength AddObject(const std::string& id, const MeshObject& obj)
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

        mLog.Debug() << __FUNCTION__ << " " << id << " off: " << mOffset << " len: " << length << std::endl;

        mOffset += obj.GetNumVertices();

        return offsetAndLength;
    }

    OffsetAndLength GetObject(const std::string& id)
    {   
        assert(mObjects.find(id) != mObjects.end());
        return mObjects.find(id)->second;
    }

//private:
    unsigned long mOffset;
    std::unordered_map<std::string, OffsetAndLength> mObjects;

    std::vector<glm::vec3> mVertices;
    std::vector<glm::vec3> mNormals;
    std::vector<glm::vec3> mColors;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<glm::vec3> mTextureBlends;
	std::vector<unsigned> mIndices;

    const Logging::Logger& mLog{
        Logging::LogState::GetLogger("MeshObjectStore")};
};

}
