#include "bak/spells.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/font.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    Logging::LogState::Disable("DialogStore");
    
    BAK::SpellInfo();
    for (unsigned i = 1; i < 7; i++)
        BAK::SymbolCoordinates{i};

    BAK::PowerRing{};

    return 0;
}

