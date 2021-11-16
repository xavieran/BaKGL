#include "bak/fmap.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("LoadZoneRef");
    BAK::FMapXY();
    BAK::FMapTowns();

    return 0;
}
