#include "bak/state/skill.hpp"

#include "bak/file/fileBuffer.hpp"

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

void SetSkillSelected(FileBuffer& fb, unsigned character, unsigned skill, bool enabled) 
{
    constexpr auto maxSkills = 0x11;
    SetEventFlag(
        fb,
        sSkillSelectedEventFlag
            + (character * maxSkills)
            + skill,
        enabled);
}

void SetSkillUnseenImprovement(FileBuffer& fb, unsigned character, unsigned skill, bool enabled)
{
    constexpr auto maxSkills = 0x11;
    SetEventFlag(
        fb,
        sSkillImprovementEventFlag
            + (character * maxSkills)
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
