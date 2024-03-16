#include "bak/book.hpp"

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);

    if (argc < 2)
    {
        std::cerr << "No arguments provided!\nUsage: "
            << argv[0] << " BOK\n";
        return -1;
    }
    
    std::string bokFile{argv[1]};

    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(bokFile);
    auto book = BAK::LoadBook(fb);
    logger.Info() << book << "\n";

    return 0;
}

