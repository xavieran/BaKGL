#include "bak/fmap.hpp"

#include "bak/file/packedResourceFile.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("LoadZoneRef");
    BAK::FMapXY();
    BAK::FMapTowns();

    return 0;
}
