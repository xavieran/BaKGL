#pragma once

#include "bak/types.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace BAK {
class FileBuffer;
}

namespace BAK::Encounter {

class Encounter;
class EncounterFactory;

std::vector<Encounter> LoadEncounters(
    const EncounterFactory&,
    FileBuffer& fb,
    Chapter chapter,
    glm::uvec2 tile,
    unsigned tileIndex);

class EncounterStore
{
public:
    EncounterStore(
        const EncounterFactory&,
        FileBuffer& fb,
        glm::uvec2 tile,
        unsigned tileIndex);
    
    const std::vector<Encounter>& GetEncounters(
        Chapter chapter) const;
    
private:
    std::vector<
        std::vector<Encounter>> mChapters;
};


}
