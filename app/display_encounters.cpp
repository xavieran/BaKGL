#include "com/logger.hpp"

#include "bak/encounter/encounter.hpp"
#include "bak/zoneReference.hpp"

#include "xbak/FileBuffer.h"

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Info);
    
    const auto ef = BAK::Encounter::EncounterFactory{};
    for (unsigned zone = 1; zone < 12; zone++)
    {
        auto zoneLabel = BAK::ZoneLabel{zone};
        const auto tiles = BAK::LoadZoneRef(zoneLabel.GetZoneReference());

        for (unsigned tileIndex = 0; tileIndex < tiles.size(); tileIndex++)
        {
            const auto& tile = tiles[tileIndex];
            const auto x = tile.x;
            const auto y = tile.y;
            try 
            {
                auto fb = FileBufferFactory::CreateFileBuffer(zoneLabel.GetTileData(x ,y));
                for (unsigned c = 1; c < 11; c++)
                {
                    logger.Info() << "Loading world tile:" << zone << " " << x 
                        << "," << y << " c: " << c << "\n";
                        const auto encounters = BAK::Encounter::LoadEncounters(
                                ef, fb, c, glm::vec<2, unsigned>(x, y), tileIndex);
                        for (const auto& encounter : encounters)
                            logger.Info() << "Encounter: " << encounter << "\n";
                }
            }
            catch (const std::exception& e)
            {
                continue;
            }
        }
    }

    


    return 0;
}

