#pragma once

#include "glm/fwd.hpp"
#include <glm/glm.hpp>

#include <array>
#include <iosfwd>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Graphics { class MeshObject; }

namespace BAK {

class FileBuffer;

constexpr unsigned EF_TERRAIN   = 0x00;
constexpr unsigned EF_UNBOUNDED = 0x20;
constexpr unsigned EF_2D_OBJECT = 0x40;

struct ClipPoint
{
    glm::ivec2 mNormal;
    glm::ivec2 mXY;
};

struct ClipElement
{
    std::vector<ClipPoint> mPoints;
    std::optional<ClipPoint> mHeightPoint;
    std::uint8_t mScale;
    std::uint16_t mBaseHeight;
};

struct ModelClip
{
    glm::ivec2 mRadius;
    std::uint8_t mFlags;
    bool mWalkable;
    bool mHasVertical;
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
    std::uint8_t mTerrainType;
    unsigned mScale;
    unsigned mSprite;
    glm::ivec3 mMin;
    glm::ivec3 mMax;
    glm::ivec3 mPos;
    std::vector<glm::i32vec3> mVertices;
    std::vector<Component> mComponents;
};

std::vector<std::string> LoadModelNames(FileBuffer& fb);
std::vector<ModelClip> LoadModelClip(FileBuffer& fb, const std::vector<std::string>& names);
std::vector<Model> LoadModels(FileBuffer& fb, const std::vector<std::string>& itemNames);
std::pair<std::vector<Model>, std::vector<ModelClip>> LoadTBL(FileBuffer& fb);

std::ostream& operator<<(std::ostream& os, const ClipPoint& cp);
std::ostream& operator<<(std::ostream& os, const ClipElement& ce);
std::ostream& operator<<(std::ostream& os, const ModelClip& mc);
}
