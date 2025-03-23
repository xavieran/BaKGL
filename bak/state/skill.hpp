#pragma once

#include <cstdint>

namespace BAK {
class FileBuffer;
}

namespace BAK::State {

bool ReadSkillSelected(FileBuffer&, unsigned character, unsigned skill);
bool ReadSkillUnseenImprovement(FileBuffer&, unsigned character, unsigned skill);
void ClearUnseenImprovements(FileBuffer&, unsigned character);

void SetSkillSelected(FileBuffer&, unsigned character, unsigned skill, bool enabled);
void SetSkillUnseenImprovement(FileBuffer&, unsigned character, unsigned skill, bool enabled);

std::uint8_t ReadSelectedSkillPool(FileBuffer&, unsigned character);
void SetSelectedSkillPool(FileBuffer&, unsigned character, std::uint8_t value);

}
