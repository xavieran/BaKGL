#include "bak/trap.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    
    logger.Info() << "Loading Traps\n";

    BAK::LoadTraps();
    return 0;
}

