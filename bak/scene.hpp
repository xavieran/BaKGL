#pragma once

#include "bak/sceneData.hpp"

#include "xbak/FileBuffer.h"

#include <optional>
#include <vector>
#include <unordered_map>

namespace BAK {

struct SceneChunk
{
    Actions mAction;
    std::optional<std::string> mResourceName;
    std::vector<std::int16_t> mArguments;
};

struct SceneIndex
{
    std::string mSceneTag;
    // Actually it's more complicated than this...
    unsigned mSceneIndex;
};

struct Scene
{
    std::string mSceneTag;
    std::vector<SceneAction> mActions;
    std::unordered_map<unsigned, std::string> mPalettes;
    std::unordered_map<unsigned, std::pair<std::string, unsigned>> mImages;

    std::optional<ClipRegion> mClipRegion;
};

std::ostream& operator<<(std::ostream&, const Scene&);

std::unordered_map<unsigned, SceneIndex> LoadSceneIndices(FileBuffer& fb);
std::unordered_map<unsigned, Scene> LoadScenes(FileBuffer& fb);
    
}
