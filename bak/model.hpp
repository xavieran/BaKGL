#pragma once

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

#include "glm/fwd.hpp"
#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <vector>

namespace BAK {

constexpr unsigned EF_TERRAIN   = 0x00;
constexpr unsigned EF_UNBOUNDED = 0x20;
constexpr unsigned EF_2D_OBJECT = 0x40;

struct ModelClip
{
    unsigned xradius;
    unsigned yradius;
    unsigned flags;
    unsigned extras;
    char extraFlag;
    std::vector<glm::ivec2> textureCoords;
    std::vector<glm::ivec2> otherCoords;
};

struct ClipPoint
{
    glm::ivec2 mUV;
    glm::ivec2 mXY;
};

struct ClipElement
{
    std::vector<ClipPoint> mPoints;
    std::optional<ClipPoint> mExtraPoint;
    std::array<std::uint8_t, 3> mUnknown;
};

struct ModelClipX
{
    glm::ivec2 mRadius;
    bool hasVertical;
    std::vector<ClipElement> mElements;
    std::string mName;
};

struct FaceOption
{
    unsigned mFaceType;
    unsigned mEdgeCount;
    std::vector<glm::vec<4, std::uint8_t>> mFaceColors;
    std::vector<std::uint8_t> mPalettes;
    std::vector<std::vector<std::uint16_t>> mFaces;
};

struct Mesh
{
    std::vector<FaceOption> mFaceOptions;
};

struct Component
{
    std::vector<Mesh> mMeshes;
};

struct Model
{
    std::string mName;
    unsigned mEntityFlags;
    unsigned mEntityType;
    unsigned mTerrainType;
    unsigned mScale;
    unsigned mSprite;
    glm::ivec3 mMin;
    glm::ivec3 mMax;
    glm::ivec3 mPos;
    std::vector<glm::i32vec3> mVertices;
    std::vector<Component> mComponents;
};

std::vector<std::string> LoadModelNames(FileBuffer& fb);
std::vector<ModelClipX> LoadModelClip(FileBuffer& fb, unsigned numItems);
std::vector<Model> LoadTBL(FileBuffer& fb);

}
