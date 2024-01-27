#include "bak/fixedObject.hpp"

#include "com/logger.hpp"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("DialogStore");
    
    BAK::GetObjectIndex();
    std::string zone{argv[1]};

    logger.Info() << "Loading fixed objects from zone: " << zone << std::endl;

    //const auto obj = BAK::LoadFixedObjects(std::atoi(zone.c_str()));

    //for (const auto& o : obj)
    //{
    //    logger.Info() << o << "\n";
    //}
    //logger.Info() << "Done\n";

    return 0;
}

