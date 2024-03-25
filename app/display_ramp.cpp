#include "bak/ramp.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::SetLogTime(false);

    auto file = std::string(argv[1]);
    auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(file);
    auto ramp = BAK::LoadRamp(fb);
    logger.Info() << ramp << "\n";
    return 0;
}
