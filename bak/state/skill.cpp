#include "bak/state/skill.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

namespace BAK::State {


bool ReadSkillSelected(FileBuffer& fb, unsigned character, unsigned skill) 
{
    constexpr auto maxSkills = 0x11;
    return ReadEventBool(
        fb,
        sSkillSelectedEventFlag
        + (character * maxSkills)
        + skill);
}

bool ReadSkillUnseenImprovement(FileBuffer& fb, unsigned character, unsigned skill) 
{
    constexpr auto maxSkills = 0x11;
    return ReadEventBool(
        fb,
        sSkillImprovementEventFlag
        + (character * maxSkills)
        + skill);
}

std::uint8_t ReadSelectedSkillPool(FileBuffer& fb, unsigned character) 
{
    fb.Seek(sCharacterSelectedSkillPool + (1 << character));
    return fb.GetUint8();
}

void SetSelectedSkillPool(FileBuffer& fb, unsigned character, std::uint8_t value)
{
    fb.Seek(sCharacterSelectedSkillPool + (1 << character));
    return fb.PutUint8(value);
}

void ClearUnseenImprovements(FileBuffer& fb, unsigned character)
{
    constexpr auto maxSkills = 0x11;
    for (unsigned i = 0; i < maxSkills; i++)
    {
        const auto flag = 
            sSkillImprovementEventFlag
            + (character * maxSkills)
            + i;

        SetEventFlagFalse(fb, flag);
    }
}

}
