#include "bak/fixedObject.hpp"

#include "bak/container.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("PackedFileDataProvider");
    
    const auto obj = BAK::LoadFixedObjects(1);

    return 0;
}

