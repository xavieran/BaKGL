#include "bak/hotspot.hpp"
#include "bak/dialog.hpp"

#include "com/logger.hpp"

#include "bak/fileBuffer.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("LoadScenes");
    Logging::LogState::Disable("LoadSceneIndices");
    
    std::string gdsFile{argv[1]};

    logger.Info() << "Loading gds:" << gdsFile << std::endl;

    auto fb = BAK::FileBufferFactory::Get().Get().CreateDataBuffer(gdsFile);
    BAK::SceneHotspots sceneHotspots{std::move(fb)};
    const auto& dialog = BAK::DialogStore::Get();
    if (sceneHotspots.mFlavourText != 0)
    {
        logger.Info() << dialog.GetFirstText(dialog.GetSnippet(BAK::KeyTarget{sceneHotspots.mFlavourText})) << std::endl;
    }

    return 0;
}

