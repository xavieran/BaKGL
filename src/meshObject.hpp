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
        mIndices{}
    {
    }

    void LoadFromBaKItem(const WorldItem& item, const Palette& pal)
    {
        auto glmVertices = std::vector<glm::vec3>{};

        for (const auto& vertex : item.GetDatItem().mVertices)
        {
            glmVertices.emplace_back(
                vertex.GetX() / 2000., 
                vertex.GetY() / 2000.,
                vertex.GetZ() / 2000.);
                //vertex.GetY() / 2000., 
                //vertex.GetZ() / 2000.);
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

                auto normal = glm::normalize(
                    glm::cross(
                        glmVertices[i_a] - glmVertices[i_b],
                        glmVertices[i_a] - glmVertices[i_c]));

                mNormals.emplace_back(normal);
                mNormals.emplace_back(normal);
                mNormals.emplace_back(normal);
                
                auto color = pal.GetColor(item.GetDatItem().mColors[i]);
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
        std::copy(obj.mIndices.begin(), obj.mIndices.end(), std::back_inserter(mIndices));

        mLog.Info() << __FUNCTION__ << " " << id << " off: " << mOffset << " len: " << length << std::endl;

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
	std::vector<unsigned> mIndices;

    const Logging::Logger& mLog{
        Logging::LogState::GetLogger("MeshObjectStore")};
};

}
