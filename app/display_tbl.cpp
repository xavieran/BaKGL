#include "bak/model.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    std::string tbl{argv[1]};

    logger.Info() << "Loading TBL:" << tbl << std::endl;

    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(tbl);
    LoadTBL(fb);
    return 0;
}
