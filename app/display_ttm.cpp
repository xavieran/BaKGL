#include "bak/scene.hpp"

#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    if (argc < 3)
    {
        std::cerr << "No arguments provided!\nUsage: "
            << argv[0] << " ADS TTM\n";
        return -1;
    }
    
    std::string adsFile{argv[1]};
    std::string ttmFile{argv[2]};

    logger.Info() << "Loading ADS and TTM:" << adsFile << " " << ttmFile << std::endl;

    {
        auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(adsFile);
        BAK::LoadSceneIndices(fb);
    }

    {
        auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(ttmFile);
        BAK::LoadScenes(fb);
    }

    return 0;
}

