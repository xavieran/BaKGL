#include "bak/scene/scene.hpp"
#include "bak/scene/sceneData.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    Logging::LogState::SetLevel(Logging::LogLevel::Always);

    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <ads|sequences|ttm|dynamic> FILE\n";
        return -1;
    }

    const std::string mode{argv[1]};
    const std::string file{argv[2]};

    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(file);

    if (mode == "ads")
    {
        for (const auto& [index, sceneIndex] : BAK::LoadSceneIndices(fb))
        {
            std::cout << index << " " << sceneIndex << "\n";
        }
    }
    else if (mode == "sequences")
    {
        for (const auto& [index, sequences] : BAK::LoadSceneSequences(fb))
        {
            for (const auto& sequence : sequences)
            {
                std::cout << index << " " << sequence.mName << " [";
                for (const auto& scene : sequence.mScenes)
                {
                    std::cout << " (" << scene.mInitScene
                        << "," << scene.mDrawScene << ")";
                }
                std::cout << " ]\n";
            }
        }
    }
    else if (mode == "ttm")
    {
        for (const auto& [index, scene] : BAK::LoadScenes(fb))
        {
            std::cout << index << " " << scene << "\n";
        }
    }
    else if (mode == "dynamic")
    {
        for (const auto& action : BAK::LoadDynamicScenes(fb))
        {
            std::cout << action << "\n";
        }
    }
    else
    {
        std::cerr << "Unknown mode: " << mode << "\n";
        return -1;
    }

    return 0;
}
