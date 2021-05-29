#pragma once

#include "src/sceneData.hpp"

#include "xbak/FileBuffer.h"

#include <vector>

namespace BAK {

struct Scene
{
    SetScene mScene;
    std::vector<SceneAction> mActions;
};

std::vector<Scene> LoadScenes(FileBuffer& fb);
    
}
