#include "bak/encounter/combat.hpp"

#include "com/logger.hpp"

int main()
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Info);

    BAK::Encounter::GenericCombatFactory<false> combatFactory;
    logger.Info() << "=== DEF_COMB.DAT (" << combatFactory.size() << " combats) ===" << std::endl;
    for (unsigned i = 0; i < combatFactory.size(); i++)
        logger.Info() << i << ": " << combatFactory.Get(i) << std::endl;

    BAK::Encounter::GenericCombatFactory<true> trapFactory;
    logger.Info() << "\n=== DEF_TRAP.DAT (" << trapFactory.size() << " traps) ===" << std::endl;
    for (unsigned i = 0; i < trapFactory.size(); i++)
        logger.Info() << i << ": " << trapFactory.Get(i) << std::endl;

    return 0;
}
