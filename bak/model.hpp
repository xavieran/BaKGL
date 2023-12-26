#pragma once

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

#include "glm/fwd.hpp"
#include <glm/glm.hpp>

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
    std::vector<std::uint8_t> mFaceColors;
    std::vector<std::uint8_t> mPalettes;
    std::vector<std::vector<std::uint16_t>> mFaces;
};


struct Mesh
{
    std::vector<std::uint8_t> mFaceColors;
    std::vector<std::uint8_t> mPalettes;
    std::vector<std::vector<std::uint16_t>> mFaces;
};

struct Component
{
    std::vector<Mesh> mMeshes;
};

struct NewModel
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

std::vector<Model> LoadTBL(FileBuffer& fb);

}
