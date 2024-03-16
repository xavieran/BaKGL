#include "bak/scene.hpp"

#include "com/logger.hpp"

#include "bak/file/fileBuffer.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    if (argc < 2)
    {
        std::cerr << "No arguments provided!\nUsage: "
            << argv[0] << " TTM\n";
        return -1;
    }
    
    std::string ttmFile{argv[1]};

    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    auto newFB = BAK::DecompressTTM(fb);

    auto saveFile = std::ofstream{
        "NEW.TTM",
        std::ios::binary | std::ios::out};
    newFB.Save(saveFile);

    return 0;
}

