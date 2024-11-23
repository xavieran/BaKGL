#include "bak/monster.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    Logging::LogState::Disable("PackedFileDataProvider");
    
    const auto& monsters = BAK::MonsterNames::Get();
    for (unsigned i = 0; i < monsters.size(); i++)
    {
        logger.Info() << "#" << i << " " << 
            monsters.GetMonsterName(BAK::MonsterIndex{i}) << " " << monsters.GetMonster(BAK::MonsterIndex{i}) << "\n";
    }
    return 0;
}

