#include "com/logger.hpp"

#include "bak/encounter/encounter.hpp"

#include "bak/palette.hpp"

#include "graphics/glm.hpp"

#include "xbak/FileBuffer.h"
#include "xbak/FileManager.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Info);
    
    const auto ef = BAK::Encounter::EncounterFactory{};
    for (unsigned zone = 1; zone < 12; zone++)
    {
        auto zoneLabel = BAK::ZoneLabel{zone};
        for (unsigned x = 9; x < 24; x++)
        {
            for (unsigned y = 9; y < 24; y++)
            {
                try
                {
                    auto fb = FileBufferFactory::CreateFileBuffer(
                        zoneLabel.GetTileData(x ,y));
                    for (unsigned c = 1; c < 11; c++)
                    {
                        logger.Info() << "Loading world tile:" << zone << " " << x 
                            << "," << y << " c: " << c << "\n";
                        const auto encounters = BAK::Encounter::LoadEncounters(
                                ef, fb, c, glm::vec<2, unsigned>(x, y));
                        for (const auto& encounter : encounters)
                            logger.Info() << "Encounter: " << encounter << "\n";
                    }
                }
                catch (const OpenError&)
                {
                }
            }
        }
    }

    


    return 0;
}

