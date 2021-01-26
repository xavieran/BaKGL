#pragma once

#include "constants.hpp"

#include "logger.hpp"

#include "worldFactory.hpp"

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
        auto glmVertices = std::vector<glm::vec3>{};

        const auto TextureBlend = [&](auto blend)
        {
            mTextureBlends.emplace_back(blend);
            mTextureBlends.emplace_back(blend);
            mTextureBlends.emplace_back(blend);
        };

        for (const auto& vertex : item.GetVertices())
        {
            glmVertices.emplace_back(
                glm::cast<float>(vertex) / BAK::gWorldScale);
        }

        for (const auto& faceV : item.GetFaces() | boost::adaptors::indexed())
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
                // textures quads anyway... (not true...)
                auto colorIndex = item.GetColors()[i];
                auto paletteIndex = item.GetPalettes()[i];
                auto textureIndex = colorIndex;

                float u = 1.0;
                float v = 1.0;

                auto maxDim = store.GetMaxDim();
                
                // I feel like these "palettes" are probably collections of
                // flags?
                static constexpr std::uint8_t texturePalette0 = 0x90;
                static constexpr std::uint8_t texturePalette1 = 0x91;
                static constexpr std::uint8_t texturePalette2 = 0xd1;
                // texturePalette3 is optional and puts grass on mountains
                static constexpr std::uint8_t texturePalette3 = 0x81;
                static constexpr std::uint8_t texturePalette4 = 0x11;
                static constexpr std::uint8_t terrainPalette = 0xc1;

                // terrain palette
                // 0 = ground
                // 1 = road
                // 2 = waterfall
                // 3 = path
                // 4 = dirt/field
                // 5 = river
                // 6 = sand
                // 7 = riverbank
                if (item.GetName().substr(0, 2) == "t0")
                {
                    if (textureIndex == 1)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Road);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 2)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Path);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 3)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                        TextureBlend(1.0);
                    }
                    else
                    {
                        TextureBlend(0.0);
                    }
                }
                else if (item.GetName().substr(0, 2) == "r0")
                {
                    if (textureIndex == 3)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 5)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Bank);
                        TextureBlend(1.0);
                    }
                    else
                    {
                        TextureBlend(0.0);
                    }
                }
                else if (item.GetName().substr(0, 2) == "g0")
                {
                    if (textureIndex == 0)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Ground);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 5)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                        TextureBlend(1.0);
                    }
                    else
                    {
                        TextureBlend(0.0);
                    }
                }
                else if (item.GetName().substr(0, 5) == "field")
                {
                    if (textureIndex == 1)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Dirt);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 2)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Bank);
                        TextureBlend(1.0);
                    }
                    else
                    {
                        TextureBlend(1.0);
                    }
                }
                else if (item.GetName().substr(0, 4) == "fall"
                    || item.GetName().substr(0, 6) == "spring")
                {
                    if (textureIndex == 3)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::River);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 5)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Bank);
                        TextureBlend(1.0);
                    }
                    else if (textureIndex == 6)
                    {
                        textureIndex = store.GetTerrainOffset(BAK::Terrain::Waterfall);
                        TextureBlend(1.0);
                    }
                    else
                    {
                        TextureBlend(0.0);
                    }
                }
                else if (paletteIndex == terrainPalette)
                {
                    textureIndex += store.GetTerrainOffset(BAK::Terrain::Ground);
                    TextureBlend(1.0);
                }
                else if (paletteIndex == texturePalette0
                    || paletteIndex == texturePalette1
                    || paletteIndex == texturePalette2
                    || paletteIndex == texturePalette4)
                {
                    TextureBlend(1.0);
                }
                else
                {
                    TextureBlend(0.0);
                }

                if (colorIndex < store.GetTextures().size())
                {
                    u = static_cast<float>(store.GetTexture(textureIndex).mWidth - 1) 
                        / static_cast<float>(maxDim);
                    v = static_cast<float>(store.GetTexture(textureIndex).mHeight - 1) 
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

                // The normal must be inverted to account
                // for the Y direction being negated
                auto normal = glm::normalize(
                    glm::cross(
                        glmVertices[i_a] - glmVertices[i_c],
                        glmVertices[i_a] - glmVertices[i_b]));

                mNormals.emplace_back(normal);
                mNormals.emplace_back(normal);
                mNormals.emplace_back(normal);
                
                auto color = pal.GetColor(colorIndex);
                auto glmCol = \
                    glm::vec4(
                        static_cast<float>(color.r) / 256,
                        static_cast<float>(color.g) / 256,
                        static_cast<float>(color.b) / 256,
                        1);

                // bitta fun
                if (item.GetName().substr(0, 5) == "cryst")
                    glmCol.a = 0.8;

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
    std::vector<glm::vec4> mColors;
    std::vector<glm::vec3> mTextureCoords;
    // Ratio of texture to material color 
    // - solution to having textured and
    // non-textured objects (should I use diff shaders?)
    std::vector<float> mTextureBlends;
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
    std::vector<glm::vec4> mColors;
    std::vector<glm::vec3> mTextureCoords;
    std::vector<float> mTextureBlends;
    std::vector<unsigned> mIndices;

    const Logging::Logger& mLog{
        Logging::LogState::GetLogger("MeshObjectStore")};
};

}
