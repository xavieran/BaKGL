#include "bak/state/skill.hpp"

#include "bak/file/fileBuffer.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

namespace BAK::State {


bool ReadSkillSelected(FileBuffer& fb, unsigned character, unsigned skill) 
{
    return ReadEventBool(
        fb,
        sSkillSelectedEventFlag
        + (character * sMaxSkills)
        + skill);
}

bool ReadSkillUnseenImprovement(FileBuffer& fb, unsigned character, unsigned skill) 
{
    return ReadEventBool(
        fb,
        sSkillImprovementEventFlag
        + (character * sMaxSkills)
        + skill);
}

void SetSkillSelected(FileBuffer& fb, unsigned character, unsigned skill, bool enabled) 
{
    SetEventFlag(
        fb,
        sSkillSelectedEventFlag
            + (character * sMaxSkills)
            + skill,
        enabled);
}

void SetSkillUnseenImprovement(FileBuffer& fb, unsigned character, unsigned skill, bool enabled)
{
    SetEventFlag(
        fb,
        sSkillImprovementEventFlag
            + (character * sMaxSkills)
            + skill,
        enabled);
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
    for (unsigned i = 0; i < sMaxSkills; i++)
    {
        const auto flag = 
            sSkillImprovementEventFlag
            + (character * sMaxSkills)
            + i;

        SetEventFlagFalse(fb, flag);
    }
}

}
