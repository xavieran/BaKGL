#include "bak/fmap.hpp"

#include "bak/packedResourceFile.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("LoadZoneRef");
    //BAK::FMapXY();
    //BAK::FMapTowns();
    BAK::ResourceIndex();

    return 0;
}
