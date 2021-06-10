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

struct Scene
{
    std::string mSceneTag;
    std::vector<SceneAction> mActions;
    std::unordered_map<unsigned, std::string> mPalettes;
    std::unordered_map<unsigned, std::string> mImages;

    template <typename T>
    const T& GetFirstAction() const
    {
        const auto& it = std::find_if(
            mActions.begin(), mActions.end(),
            [](const auto& a){ return std::holds_alternative<T>(a);});
        assert(it != mActions.end());
        return std::get<T>(*it);
    }
};

std::ostream& operator<<(std::ostream&, const Scene&);

std::vector<Scene> LoadScenes(FileBuffer& fb);
    
}
